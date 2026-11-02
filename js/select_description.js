/**
 * @file
 * Utility function to display selected entries in multiple selection elements.
 */

(function ($) {

Backdrop.behaviors.updateSelectDescription = {
  attach: function (context, settings) {
    // Target all multiple selects on the discount edit form.
    $('.uc-discounts-update-select-description', context).once('update-desc').each(function () {
      var $select = $(this);

      // Find the wrapper element or description field relative to this select element
      var $desc = $select.closest('.form-item').find('.description');

      // Helper function to update the DOM text
      function updateSelectedText() {
        var selected = [];

        $select.find('option:selected').each(function () {
          selected.push($(this).text());
        });

        // Ensure container div exists inside .description
        var $displayContainer = $desc.find('.uc-discounts-selected-list');
        if ($displayContainer.length === 0) {
          $displayContainer = $('<div class="uc-discounts-selected-list"></div>').appendTo($desc);
        }

        if (selected.length > 0) {
          $displayContainer.text(Backdrop.t('Currently selected: ') + selected.join(', '));
        } else {
          $displayContainer.text(Backdrop.t('No items selected.'));
        }
      }

      // Run once on page initialization (for pre-selected default values)
      updateSelectedText();

      // Bind update handler to the change event
      $select.on('change', updateSelectedText);
    });
  }
};

})(jQuery);
