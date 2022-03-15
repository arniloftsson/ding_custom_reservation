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
  $from_date = date("Y-m-d");
  $to_date = date("Y-m-d");

  if (isset($_SESSION['ding_custom_reservation_statistics'])) {
    if (isset($_SESSION['ding_custom_reservation_statistics']['from'])) {
      $from_date = $_SESSION['ding_custom_reservation_statistics']['from'];
    }
    if (isset($_SESSION['ding_custom_reservation_statistics']['to'])) {
      $to_date = $_SESSION['ding_custom_reservation_statistics']['to'];
    }
  }


  $form['period'] = array(
    '#type' => 'fieldset',
    '#title' => t('Period'),
  );

  $form['period']['from_date'] = array(
    '#type' => 'date',
    '#title' => t('From'),
    '#default_value' => $from_date,
  );

  $form['period']['to_date'] = array(
    '#type' => 'date',
    '#title' => t('To'),
    '#default_value' => $to_date,
    '#description' => t('Enter custom period for reservation statistics.'),
  );

  $form['period']['submit_period'] = array(
    '#value' => t('Filter by date'),
    '#type' => 'submit',
    // '#validate' => array('ting_smart_search_import_form_validate'),
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
    }
    if (isset($_SESSION['ding_custom_reservation_statistics']['to'])) {
      $date = $_SESSION['ding_custom_reservation_statistics']['to'];
      $to_date = mktime(0, 0, 0, $date['month'], $date['day'], $date['year']);
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
  } catch (\Throwable $th) {
    //throw $th;
  }
  return $results;
}
