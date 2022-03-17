<?php

/**
 * @file
 * Implements statistics report page for Ding Custom Reservation.
 */

/**
 * Implements hook_form().
 */
function ding_custom_reservation_statistics_form($form, &$form_state) {
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

  $form['export_statistics'] = array(
    '#type' => 'fieldset',
    '#title' => t('Export Statistics'),
    '#description' => t('Exports all entries in the statistics table to csv file'),
  );
  $form['export_statistics']['submit_button'] = [
    '#value' => t('Export Statistics'),
    '#type' => 'submit',
    '#submit' => ['ding_custom_reservation_statistics_form_export'],
  ];

  return system_settings_form($form);
}

/**
 * Filter by date validation
 */
function ding_custom_reservation_statistics_form_validate($form, &$form_state) {
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
 * Filter by date submit.
 */
function ding_custom_reservation_statistics_form_submit($form, &$form_state) {
  if (isset($form_state['values']['from_date']) && isset($form_state['values']['to_date'])) {
    $_SESSION['ding_custom_reservation_statistics'] = [
      'from' => $form_state['values']['from_date'],
      'to' => $form_state['values']['to_date'],
    ];
  }
}

/**
 * Get statistics data.
 */
function ding_custom_reservation_statistics_form_get_data() {
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
 * Get statistics from database.
 */
function ding_custom_reservation_statistics_form_db_query($reservation_type, $from_date = null, $to_date = null) {
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

/**
 * Export statistics records.
 */
function ding_custom_reservation_statistics_form_export($form, &$form_state) {
  $path = file_build_uri('CustomReservationStatistics-' . date('d-m-Y-H-i') . '.csv');
  $destination = drupal_realpath($path);

  try {
    $data = ding_custom_reservation_statistics_get_all_records();

    if (isset($data)) {
      $csv_file = fopen($destination, 'w');
      $file_header = ['Interesseperiode', 'Reserveringstype', 'Materialetype', 'Dato'];
      fputcsv($csv_file,  $file_header, ',', '"');
      foreach ($data as $record) {
        fputcsv($csv_file, $record, ',', '"');
      }
      fclose($csv_file);

      $headers = array('Content-Disposition' => 'attachment; filename="' . $path  . '"');
      file_transfer($path, $headers);
    }

  } catch (Exception $e) {
    watchdog_exception('ding_custom_reservation', $e, 'Export off records from database failed');
    drupal_set_message('Export off records from database failed', 'error');
  }
}

/**
 * Retrieve all records in statistics table from database.
 */
function ding_custom_reservation_statistics_get_all_records() {
  $results =  [];

  $query = db_select('ding_custom_reservation_statistics', 'd')
    ->fields('d', [
      'reservationtype',
      'interestperiod',
      'materialtype',
      'day',
    ]);

  $result = $query->execute();

  while ($data = $result->fetchObject()) {
    $day = date('d-m-Y', $data->day);
    $reservation_type = 'primary';
    if ($data->reservationtype == DING_CUSTOM_RESERVATION_SECONDARY_BUTTON) {
      $reservation_type = 'secondary';
    }
    $results[] = [$data->interestperiod, $reservation_type, $data->materialtype, $day];
  };

  return $results;
}
