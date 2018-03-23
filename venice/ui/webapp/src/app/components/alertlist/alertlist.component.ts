import { Component, EventEmitter, Input, OnChanges, OnDestroy, OnInit, Output, ViewEncapsulation } from '@angular/core';

import { Utility } from '../../common/Utility';

/**
 * This component shows up in VeniceUI right-hand-side navigation panel when user click on "alert" toolbar icon
 *
 * app.component.ts will invoke this component in app.component.html.  app.component will inject @Input data to this component.
 * AlertlistComponent internally manage a list of AlertlistitemComponent (alert)
 * AlertlistComponent will also control the pointing up-arrow and filter display alert item by severity according to alert severity
 *
 */
@Component({
  selector: 'app-alertlist',
  templateUrl: './alertlist.component.html',
  styleUrls: ['./alertlist.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class AlertlistComponent implements OnInit, OnDestroy, OnChanges {
  @Input() data: Array<any>;
  @Output() expandAllAlertsClick: EventEmitter<any> = new EventEmitter();

  protected alertnumber = {
    total: 0,
    critical: 0,
    warning: 0,
    info: 0
  };

  protected menuIndicatorCSS = {};
  protected alerts = [];
  protected currentAlertFilterType = 'total';

  constructor() { }

  /**
   * Component life cycle API
   */
  ngOnInit() {
    this._build_menuIndicatorCSS(20);
  }

  /**
   * Component life cycle API
   */
  ngOnDestroy() {
  }

  /**
   * If input source changes, the API will get invoked.
   */
  ngOnChanges() {
    if (this.data && Array.isArray(this.data)) {
      this.alerts = this.data;
      this.alertnumber = Utility.computeAlertNumbers(this.alerts);
    }
  }

  isToShowThisAlert(alert): boolean {
    const alertType = this.currentAlertFilterType;
    if (alertType === 'total') {
      return true;
    } else {
      return (alert.severity === alertType);
    }
  }

  /**
   * This API serves html template.
   * It will filter alerts displayed in list
   */
  onAlertNumberClick(event, alertType: string) {
    this.moveMenuIndicator(event.currentTarget);
    this.currentAlertFilterType = alertType;
  }

  /**
   * This function animates the up-arrow
   */
  moveMenuIndicator(element: any) {
    const $ = Utility.getJQuery();
    const position = $(element).position();
    // Get the menu item's position X
    const posX = position.left;
    // Get the center offset (+5 to compensate the left-rignt margin 10px defined in CSS)
    const offset = $(element).width() / 2 + 12;
    // Move the triangle(corss browser)
    const distance = posX + offset;
    this._build_menuIndicatorCSS(distance);

  }

  /**
   * This funciton build to animation CSS for up-arrow
   */
  protected _build_menuIndicatorCSS(distance: number) {
    const translate = 'translate3d(' + (distance) + 'px,0,0)';
    const css = {
      'transform': translate,
      '-webkit-transform': translate,
      '-o-transform': translate,
      '-ms-transform': translate,
      '-moz-transform': translate
    };
    this.menuIndicatorCSS = css;
  }

  /**
   * AlertlistItem widget emits alert click event.
   * We should update the alert in
   * @param alert
   */
  onAlertClick(changedAlert) {
    // console.log("AlertlistComponent.onAlertClick() alert " + changedAlert.id + " is acknowledged");
  }

  /**
   * If user clicks on "expand-all-alert icon", we send out
   *
   * @param event
   */
  onExpandAlertsClick(event) {
    this.expandAllAlertsClick.emit(this.alerts);
  }



}
