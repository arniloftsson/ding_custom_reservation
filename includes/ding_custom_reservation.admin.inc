<?php

/**
 * @file
 * Implements admin page for ding custom reservation.
 */

/**
 * Implements hook_form().
 */
function ding_custom_reservation_admin_form($form, &$form_state) {
  $form = [];

  $form['ding_custom_reservation_interest_periods'] = [
    '#type' => 'textarea',
    '#title' => t('Custom reservation interest periods'),
    '#description' => t('Adjust which interest periods can be selected by the user in their profile. The format is "days"|"label". Insert one interest period pr. line.'),
    '#default_value' => ding_custom_reservation_interest_periods_strings(),
    '#element_validate' => ['ding_custom_reservation_interest_periods_validate'],
    '#required' => TRUE,
  ];

  $form['ding_custom_reservations_interest_period'] = [
    '#type' => 'select',
    '#title' => t('Reservation button interest period'),
    '#options' => ding_custom_reservation_interest_periods(),
    '#required' => TRUE,
    '#default_value' => variable_get('ding_custom_reservations_interest_period'),
  ];

  $form['ding_custom_reservation_secondary_reservation_button'] = [
    '#type' => 'checkbox',
    '#title' => t('Secondary reservation button'),
    '#description' => t('Show secondary reservation button with alternative fixed interestperiod.'),
    '#default_value' => variable_get('ding_custom_reservation_secondary_reservation_button', FALSE),
  ];

  $form['ding_custom_reservation_secondary_reservation_button_interest_period'] = [
    '#type' => 'select',
    '#title' => t('Secondary reservation button interest period'),
    '#options' => ding_custom_reservation_interest_periods(),
    '#required' => TRUE,
    '#default_value' => variable_get('ding_custom_reservation_secondary_reservation_button_interest_period', 60),
    '#states' => [
      'visible' => [
        ':input[name="ding_custom_reservation_secondary_reservation_button"]' => ['checked' => TRUE],
      ],
    ],
  ];

  $form['ding_custom_reservation_secondary_reservation_button_text'] = [
    '#type' => 'textfield',
    '#title' => t('Secondary reservation button text'),
    '#default_value' => variable_get('ding_custom_reservation_secondary_reservation_button_text', ''),
    '#states' => [
      'visible' => [':input[name="ding_custom_reservation_secondary_reservation_button"]' => ['checked' => TRUE],],
      'required' => [':input[name="ding_custom_reservation_secondary_reservation_button"]' => ['checked' => TRUE],]
      ],
    ];

  return system_settings_form($form);
}

/**
 * Helper function for interest periods strings.
 */
function ding_custom_reservation_interest_periods_strings() {
  $periods = ding_custom_reservation_interest_periods();
  $periods_string = [];
  foreach ($periods as $key => $value) {
    $periods_string[] = "$key|$value";
  }
  return implode("\n", $periods_string);
}

/**
 * Element validate callback for interest periods.
 */
function ding_custom_reservation_interest_periods_validate($element, &$form_state) {
  $periods = explode("\n", $element['#value']);
  $periods = array_map('trim', $periods);
  $periods = array_filter($periods);

  $values = [];
  foreach ($periods as $period) {
    $matches = [];
    if (!preg_match('/(.*)\|(.*)/', $period, $matches)) {
      form_error($element, t('Each line must be a key-value pair separated by "|"'));
    }
    $key = $matches[1];
    $value = $matches[2];
    if (!preg_match('/^-?\d+$/', $key)) {
      form_error($element, t('Each key must be an integer denoting the number of days of the interest period'));
    }
    $values[$key] = $value;
  }
  form_set_value($element, $values, $form_state);
}



