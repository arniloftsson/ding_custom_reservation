(function ($) {
  "use strict";

  // The form is loaded via ajax so only after an ajax call can we set focus.
  $(document).ajaxComplete(function (e, xhr, settings) {
    $('#edit-provider-options-interest-period').focus();
  });
}(jQuery));
