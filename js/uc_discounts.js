/**
 * @file
 * Utility function to display settings summaries on vertical tabs.
 */

(function ($) {

Backdrop.behaviors.ucCartDiscountPaneSummaries = {
  attach: function (context) {
    $('fieldset#edit-pane-uc-discounts', context).backdropSetSummary(function(context) {
      var ret = [];
      if ($('#edit-uc-discounts-checkout-allow-user-codes').is(':checked')) {
        ret.push(Backdrop.t('Discount codes enabled.'));
      }
      else {
        ret.push(Backdrop.t('No discount codes.'));
      }
      if ($('#edit-uc-discounts-checkout-show-messages').is(':checked')) {
        ret.push(Backdrop.t('Show messages.'));
      }
      else {
        ret.push(Backdrop.t('No messages.'));
      }
      if ($('#edit-uc-discounts-checkout-show-in-cart-contents').is(':checked')) {
        ret.push(Backdrop.t('Show discounts in cart contents.'));
      }
      else {
        ret.push(Backdrop.t('No discounts in cart contents.'));
      }
      return ret.join('<br>');
    });
  }
};

})(jQuery);
