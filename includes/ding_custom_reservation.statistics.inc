<?php

/**
 * @file
 * Implements admin page for ding custom reservation.
 */

/**
 * Implements hook_form().
 */
function ding_custom_reservation_statistics_form($form, &$form_state)
{
  $form = [];

  $form['period'] = array(
    '#type' => 'fieldset',
    '#title' => t('Period'),
  );

  $form['period']['from_date'] = array(
    '#type' => 'date',
    '#title' => t('From'),
  );

  $form['period']['to_date'] = array(
    '#type' => 'date',
    '#title' => t('To'),
    '#description' => t('Enter custom period for reservation statistics.'),
  );

  if (isset($_SESSION['ding_custom_reservation_statistics'])) {
    if (isset($_SESSION['ding_custom_reservation_statistics']['from'])) {
      $form['period']['from_date']['#default_value'] = $_SESSION['ding_custom_reservation_statistics']['from'];
    }
    if (isset($_SESSION['ding_custom_reservation_statistics']['to'])) {
      $form['period']['to_date']['#default_value'] = $_SESSION['ding_custom_reservation_statistics']['to'];
    }
  }

  $form['period']['submit_period'] = array(
    '#value' => t('Filter by date'),
    '#type' => 'submit',
    '#validate' => array('ding_custom_reservation_statistics_form_validate'),
    '#submit' => array('ding_custom_reservation_statistics_form_submit'),
  );
  $header = [t('Interest Period'), t('Count')];

  $results = ding_custom_reservation_statistics_form_get_data();

  $primary_button = theme('table', ['header' => $header, 'rows' => $results['primary_button']]);
  $secondary_button = theme('table', ['header' => $header, 'rows' => $results['secondary_button']]);

  $form['primary_button'] = array(
    '#type' => 'fieldset',
    '#title' => t('Primary reservation button'),
  );
  $form['primary_button']['table'] = [
    '#markup' =>  $primary_button,
  ];

  $form['secondary_button'] = [
    '#type' => 'fieldset',
    '#title' => t('Secondary reservation button'),
  ];
  $form['secondary_button']['table'] = [
    '#markup' =>  $secondary_button,
  ];

  return system_settings_form($form);
}

/**
 * Statistics report filter submit.
 */
function ding_custom_reservation_statistics_form_validate($form, &$form_state)
{
  file_put_contents("/var/www/drupalvm/drupal/web/debug/stat4.txt", print_r($form, TRUE), FILE_APPEND);
  if (isset($form_state['values']['from_date']) && isset($form_state['values']['to_date'])) {
    $date = $form_state['values']['from_date'];
    $from_date = mktime(0, 0, 0, $date['month'], $date['day'], $date['year']);
    $date = $form_state['values']['to_date'];
    $to_date = mktime(0, 0, 0, $date['month'], $date['day'], $date['year']);
    if ($from_date > $to_date) {
      form_set_error('from_date', t('From date cannot be after To date.'));
    }
  }
}

/**
 * Report filter submit.
 */
function ding_custom_reservation_statistics_form_submit($form, &$form_state)
{
  if (isset($form_state['values']['from_date']) && isset($form_state['values']['to_date'])) {
    $_SESSION['ding_custom_reservation_statistics'] = [
      'from' => $form_state['values']['from_date'],
      'to' => $form_state['values']['to_date'],
    ];
  }
}

/**
 * Report filter submit.
 */
function ding_custom_reservation_statistics_form_get_data()
{
  $results = [];
  $from_date = null;
  $to_date = null;

  if (isset($_SESSION['ding_custom_reservation_statistics'])) {
    if (isset($_SESSION['ding_custom_reservation_statistics']['from'])) {
      $date = $_SESSION['ding_custom_reservation_statistics']['from'];
      $from_date = mktime(0, 0, 0, $date['month'], $date['day'], $date['year']);
      unset($_SESSION['ding_custom_reservation_statistics']['from']);
    }
    if (isset($_SESSION['ding_custom_reservation_statistics']['to'])) {
      $date = $_SESSION['ding_custom_reservation_statistics']['to'];
      $to_date = mktime(0, 0, 0, $date['month'], $date['day'], $date['year']);
      unset($_SESSION['ding_custom_reservation_statistics']['to']);
      unset($_SESSION['ding_custom_reservation_statistics']);
    }
  }

  $results['primary_button'] = ding_custom_reservation_statistics_form_db_query(DING_CUSTOM_RESERVATION_PRIMARY_BUTTON, $from_date, $to_date);
  $results['secondary_button'] = ding_custom_reservation_statistics_form_db_query(DING_CUSTOM_RESERVATION_SECONDARY_BUTTON, $from_date, $to_date);

  return $results;
}

/**
 * Report filter submit.
 */
function ding_custom_reservation_statistics_form_db_query($reservation_type, $from_date = null, $to_date = null)
{
  $results =  [];
  try {
    $query = db_select('ding_custom_reservation_statistics', 'd')
    ->fields('d', [
        'interestperiod',
    ]);
    $query->condition('reservationtype', $reservation_type, '=');
    if (isset($from_date) && isset($to_date)) {
      $query->condition('day', $from_date, '>=');
      $query->condition('day', $to_date, '<=');
    }
  
    $query->addExpression('count(interestperiod)', 'reservation_count');
    $query->groupBy("d.interestperiod");
  
    $result = $query->execute();

    $total_count = 0;
    while ($data = $result->fetchObject()) {
      $results[] = [$data->interestperiod, $data->reservation_count];
      $total_count += $data->reservation_count;
    };
    $results[] = [t('Total count'), $total_count];
  } catch (Exception $e) {
    watchdog_exception('ding_custom_reservation', $e,'Fetching statisticsdata from database failed');
    drupal_set_message('Failed to get data from the database', 'error');
  }
  return $results;
}
