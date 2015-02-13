<?php

/**
 * @file
 * Rules integration for uc_discounts module.
 */

/**
 * Implements hook_rules_condition_info().
 */
function uc_discounts_rules_condition_info() {
  $conditions = array();
  $conditions['uc_discounts_condition_total_after_discounts'] = array(
    'label' => t('Check the order total after discounts have been applied'),
    'module' => 'uc_discounts',
    'group' => t('Order'),
    'parameter' => array(
      'order' => array(
        'type' => 'uc_order',
        'label' => t('Order'),
      ),
      'order_total_value' => array(
        'type' => 'integer',
        'label' => t('Order total value after discount'),
      ),
      'operator_comparison' => array(
        'type' => 'text',
        'label' => t('Operator'),
        'options list' => 'uc_order_condition_value_operator_options',
      ),
    ),
  );

  $conditions['uc_discounts_condition_discount_applied_condition'] = array(
    'label' => t('Check if a discount has been applied to the order'),
    'module' => 'uc_discounts',
    'group' => t('Order'),
    'parameter' => array(
      'order' => array(
        'type' => 'uc_order',
        'label' => t('Order'),
      ),
      'discount_id' => array(
        'type' => 'text',
        'label' => t('Discount id'),
        'options list' => 'uc_discounts_discount_id_options',
      ),
    ),
  );
  return $conditions;
}

/**
 * Checks if a particluar discount id has been applied on an order.
 */
function uc_discounts_condition_discount_applied_condition($order, $discount_id) {
  global $user;

  $order->uc_discounts_codes = $_SESSION['uc_discounts_codes'];
  $results = uc_discounts_get_discounts_for_order($order);
  foreach ($results['discounts'] as $discount) {
    if (in_array($discount->discount_id, $discount_id)) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
 * Check the current order balance minus any discounts.
 */
function uc_discounts_condition_total_after_discounts($order, $order_total_value, $operator_comparison) {
  $order->uc_discounts_codes = $_SESSION['uc_discounts_codes'];
  $results = uc_discounts_get_discounts_for_order($order);
  $total_discount = 0;
  foreach ($results['discounts'] as $discount) {
    $total_discount += $discount->amount;
  }
  $total = uc_order_get_total($order, TRUE) - $total_discount;
  switch ($operator_comparison) {
    case 'less':
      return $total < $order_total_value;

    case 'less_equal':
      return $total <= $order_total_value;

    case 'equal':
      return $total == $order_total_value;

    case 'greater_equal':
      return $total >= $order_total_value;

    case 'greater':
      return $total > $order_total_value;
  }
}

/**
 * Option list callback for discount ids.
 */
function uc_discounts_discount_id_options() {
  $discounts = db_select('uc_discounts', 'ud')
      ->fields('ud')
      ->orderBy('name', 'asc')
      ->execute()->fetchAll();
  foreach ($discounts as $discount) {
    $options[$discount->discount_id] = $discount->name;
  }

  return $options;
}
