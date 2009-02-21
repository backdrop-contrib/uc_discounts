var uc_discountsLineItems = [];
var uc_discountsisUpdating = false;

//Handles onload calls for uc_discounts.
function uc_discountsOnLoad(context)
{
    if (context == undefined)
        context = $('body');

    uc_discountsProcessCodes(context);

	//Add click event listener to discounts pane button once
	$("input[@id*=uc-discounts-button]:not(.uc_discountsOnLoad-processed)", 
		context).addClass("uc_discountsOnLoad-processed").click(function()
		{
			uc_discountsProcessCodes(context);
			//Return false to prevent default actions and propogation
			return false;
		});
}

//Processes currently entered discounts
function uc_discountsProcessCodes(context)
{
    //If currently updating, wait
    if (uc_discountsisUpdating)
    {
        setTimeout(function() { uc_discountsProcessCodes(context); }, 200);
        return;
    }

	var parameterMap = {};
	parameterMap["uc-discounts-codes"] = $("textarea[@id*=uc-discounts-codes]", context).val();

	//Show loading container
	var progress = new Drupal.progressBar("uc_discountsProgress");
	progress.setProgress(-1, Drupal.settings.uc_discounts.progress_msg);
	var messages_container = $(".uc-discounts-messages-container");
	messages_container.empty().append(progress.element);
	messages_container.addClass("solid-border");

	$.ajax({
		type: "POST",
		url: Drupal.settings.basePath + "?q=cart/checkout/uc_discounts/calculate",
		data: parameterMap,
		complete : function(xmlHttpRequest, textStatus)
			{
                //Hide loading container
                $(".uc-discounts-messages-container").removeClass("solid-border").empty();

				//If status is not 2XX
				if ( parseInt(xmlHttpRequest.status / 100) != 2)
				{
					alert(Drupal.settings.uc_discounts.err_msg);
					return;
				}

				try { uc_discountsProcessCalculateDiscountResponse(Drupal.parseJson(xmlHttpRequest.responseText), context); }
				catch (e)
				{
					alert(Drupal.settings.uc_discounts.err_msg);
					return;
				}
			}
	});
}

//Processes calculateDiscountResponse from drupal
function uc_discountsProcessCalculateDiscountResponse(calculateDiscountResponse, context)
{
    if (uc_discountsisUpdating)
        return;
    uc_discountsisUpdating = true;

    try
    {
        var i;

        if (calculateDiscountResponse == null)
	    {
		    alert(Drupal.settings.uc_discounts.err_msg);
		    return;
	    }

        var line_items = calculateDiscountResponse[Drupal.settings.uc_discounts.calculate_discount_response_line_items_key];
        var errors = calculateDiscountResponse[Drupal.settings.uc_discounts.calculate_discount_response_errors_key];
        var messages = calculateDiscountResponse[Drupal.settings.uc_discounts.calculate_discount_response_messages_key];

        //Process discount line items and update total (false to not display messages)
        uc_discountsRenderLineItems(line_items, true);
        uc_discountsUpdateTotal();

        //Add errors and messages to messages container
        var discounts_messages_container = $(".uc-discounts-messages-container", context);
        discounts_messages_container.empty();

        if ( (errors != null) && (errors.length > 0) )
            discounts_messages_container.append(  $("<div class='uc-discounts-messages messages error'><" + "/div>").append( errors.join("<br/>") )  );
        if ( (messages != null) && (messages.length > 0) )
        {
            var message_list = $("<ul><" + "/ul>");
            for (var i = 0; i < messages.length; i++)
                message_list.append( $("<li><" + "/li>").append(messages[i]) );
            discounts_messages_container.append( $("<div class='uc-discounts-messages messages'><" + "/div>").append(message_list) );
        }

        uc_discountsisUpdating = false;
    }
    catch (e)
    {
        uc_discountsisUpdating = false;
        throw e;
    }
}

//Updates the discount line items list and updates totals
function uc_discountsRenderLineItems(line_items, show_message)
{
    uc_discountsRemoveDiscountLineItems();

    for (i = 0; i < line_items.length; i++)
    {
        var line_item = line_items[i];
        uc_discountsLineItems.push(line_item);
        if (window.set_line_item)
            set_line_item(line_item["id"], line_item["title"], line_item["amount"], line_item["weight"], 1, false);
    }
}

//Removes existing discount line items and updates totals (if updateLineItems is true)
function uc_discountsRemoveDiscountLineItems(updateLineItems)
{
    var line_items = uc_discountsLineItems;
    uc_discountsLineItems = [];
    
    //If there are no line items, there is nothing to do
    if (line_items.length == 0)
        return;

    for (i = 0; i < line_items.length; i++)
        remove_line_item(line_items[i]["id"]);
    
    if (updateLineItems)
        uc_discountsUpdateTotal();
}

//Updates total
function uc_discountsUpdateTotal()
{
    if (window.render_line_items)
        render_line_items();

    //If there is tax in the system, recalculate tax
    if (window.getTax)
        getTax();
}
