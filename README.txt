Drupal Ubercart Discounts Alternative
------------------------
Author - Ryan Groe ryan at groe dot org
License - GPL (see LICENSE)


Overview:
--------
The uc_discounts_alt module allows for coded and codeless discounts designed for 
Ubercart 2.


Upgraders:
--------
If you are upgrading from a zip version I posted on my website you will need to 
either change the module path in your database or uninstall and reinstall this 
module.


Installation:
------------
1. Place this module directory in your modules folder (this will usually be 
   "sites/all/modules/" or "sites/all/modules/ubercart").
2. Go to "administer" -> "modules" and enable the module.

How to install the codeless_discounts_field:
	Enable the module
	go to <site>/admin/content/node-type/product/fields and under "Add" "New Field" enter:
	Label: Codeless Discount
	Field name: field_codeless_discount
	Type of data: Codeless Discount
	Form element: Default Display

	Then you can reorder the fields as you like. You can also theme this 
		field (see theme_codeless_discounts_field_get_codeless_discount_html_for_product).

How to install the product_price_alterer_field:
	Enable the module
	go to <site>/admin/content/node-type/product/fields and under "Add" "New Field" enter:
	Label: Product Price Alterer
	Field name: field_product_price_alterer
	Type of data: Product Price Alterer
	Form element: Default Display

	Then you can reorder the fields as you like. You can also theme this 
		field (see theme_product_price_alterer_field_get_price_alterer_javascript_for_product).


Configuration:
-------------
1. Grant the "configure discounts" permission to the proper roles.
2. Create discounts from the menu item 
  "Store administration/Configuration/Discount settings".


Last updated:
------------
$Id$
