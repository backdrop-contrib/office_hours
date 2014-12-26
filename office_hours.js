(function ($) {
  Drupal.behaviors.office_hours = {
    attach: function(context,settings) {
      $(".oh-hide", context).parent().hide();
      $(".oh-add-more-link", context).each(function (i) {
        $(this).parent().children("div.office-hours-block").hide();
        // If the previous row has an "Add more hours" link, and the office-hours-block is hidden, don't show this line.
        $this_tr = $(this).closest("tr");
        if ($this_tr.prev().find(".oh-add-more-link").length && !$this_tr.prev().find("div.office-hours-block:visible").length) {
          $this_tr.hide();
        }
      })
      .bind('click', show_upon_click);

      fix_striping();

      // Clear the content of this block, when user clicks "Clear/Remove".
      $('.oh-clear-link').bind('click', function(e) {
        $(this).parent().parent().find('.form-select').each(function() {
          $(this).val($("#target option:first").val());
        });
        e.preventDefault();
      });

      // Show an office-hours-block, when user clicks "Add more".
      function show_upon_click(e) {
        $(this).hide();
        $(this).parent().children("div.office-hours-block").fadeIn("slow");
        e.preventDefault();

        // If the next item has an "add more" link, show it.
        $next_tr = $(this).closest("tr").next();
        if ($next_tr.find(".oh-add-more-link").length) {
          $next_tr.show();
        }
        fix_striping();
        return false;
      };

      $('.oh-apply-link-0').bind('click', function (e) {
        // Get field name.
        var $field_name = $(this).closest('.field-type-office-hours').attr('id');

        // Get field name with language and delta.
        $this_field_classes = $(this).parent().parent().children().closest('div').attr('class').split(' ');
        $($this_field_classes).each(function (i, val){
          if (/form-item-field/i.test(val)) {
            $this_field_name = val.replace('form-item-','').replace('-starthours-hours','');
          }
        });

        $('select[id^='+$field_name+'][id$="starthours-hours"]').val($('#edit-'+$this_field_name+'-starthours-hours').val());
        $('select[id^='+$field_name+'][id$="starthours-minutes"]').val($('#edit-'+$this_field_name+'-starthours-minutes').val());
        $('select[id^='+$field_name+'][id$="starthours-ampm"]').val($('#edit-'+$this_field_name+'-starthours-ampm').val());
        $('select[id^='+$field_name+'][id$="endhours-hours"]').val($('#edit-'+$this_field_name+'-endhours-hours').val());
        $('select[id^='+$field_name+'][id$="endhours-minutes"]').val($('#edit-'+$this_field_name+'-endhours-minutes').val());
        $('select[id^='+$field_name+'][id$="endhours-ampm"]').val($('#edit-'+$this_field_name+'-endhours-ampm').val());

        e.preventDefault();
      });

      // Function to traverse visible rows and apply even/odd classes.
      function fix_striping() {
        $('tbody tr:visible', context).each(function (i) {
          if (i % 2 == 0) {
            $(this).removeClass('odd').addClass('even');
          } else {
            $(this).removeClass('even').addClass('odd');
          }
        });
      }
    }
  };
 })(jQuery);
