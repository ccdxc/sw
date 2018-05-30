import { Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { AlerttableService } from 'app/services/alerttable.service';
import { Table } from 'primeng/table';

import { Eventtypes } from '../../enum/eventtypes.enum';
import { BaseComponent } from '../base/base.component';
import { Utility } from '@app/common/Utility';

/**
 * This component displays all the alerts in a primeng turbo table, and allows
 * filtering by alert type
 */
@Component({
  selector: 'app-alerttable',
  templateUrl: './alerttable.component.html',
  styleUrls: ['./alerttable.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class AlerttableComponent extends BaseComponent implements OnInit, OnDestroy {
  @ViewChild('alerttable') alertTurboTable: Table;

  alerts: any;
  cols: any[];
  selectedAlerts: any = [];
  loading = false;

  protected alertnumber = {
    total: 0,
    critical: 0,
    warning: 0,
    info: 0
  };

  constructor(protected _controllerService: ControllerService,
    protected _alerttableService: AlerttableService) {
    super(_controllerService);
  }
  /**
   * Component enters init stage. It is about to show up
   */
  ngOnInit() {
    if (!this._controllerService.isUserLogin()) {
      this._controllerService.publish(Eventtypes.NOT_YET_LOGIN, {});
    } else {
      this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'AlerttableComponent', 'state': Eventtypes.COMPONENT_INIT });
      this.getAlerts();

      this.cols = [
        { field: 'id', header: 'ID' },
        { field: 'date', header: 'Date' },
        { field: 'name', header: 'Name' },
        { field: 'message', header: 'Message' },
        { field: 'severity', header: 'Severity' }
      ];
      this._controllerService.setToolbarData({

        buttons: [
          {
            cssClass: 'global-button-primary alerttable-toolbar-refresh-button',
            text: 'Refresh',
            callback: () => { this.buttoncallback(); },
          }],
        breadcrumb: [{ label: 'Alerts', url: '' }]
      });
    }
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to exist
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'AlerttableComponent', 'state': Eventtypes.COMPONENT_DESTROY });
  }

  buttoncallback() {
    console.log(this.getClassName() + '.buttoncallback()');
    this.getAlerts();
  }

  /**
   * Overide super's API
   * It will return this Component name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  /**
   * Call server to fetch all alerts to populate RHS alert-list
   */
  getAlerts() {
    const payload = '';
    this._alerttableService.getAlerts(payload).subscribe(
      data => {
        this.alerts = data;
        this.alertnumber = Utility.computeAlertNumbers(this.alerts);
      },
      err => {
        this.successMessage = '';
        this.errorMessage = 'Failed to get items! ' + err;
        this.error(err);
      }
    );
  }

  /**
   * This api serves html template
   */
  getAlertItemIconClass(record) {
    return 'alerttablerecord-icon-' + record.severity;
  }

  /**
   * This api serves html template
   */
  onAlerttableArchiveRecord($event, alert) {
    console.log('AlerttableComponent.onAlerttableArchiveRecord()', alert);
  }

  /**
   * This api serves html template
   */
  onAlerttableDeleteRecord($event, alert) {
    console.log('AlerttableComponent.onAlerttableDeleteRecord()', alert);
  }

  /**
   * This api serves html template
   */
  onAlertClick($event, alert) {
    console.log('AlerttableComponent.onAlertClick()', alert);
  }

  /**
  * This API serves html template.
  * It will filter alerts displayed in list
  */
  onAlertNumberClick(event, alertType: string) {
    if (alertType === 'total') {
      this.alertTurboTable.filter('', 'severity', 'equals');
    } else {
      this.alertTurboTable.filter(alertType, 'severity', 'equals');
    }
  }

  /**
  * This API serves html template.
  * It will delete selected alerts
  */
  onAlerttableDeleteMultiRecords($event) {
    console.log('AlerttableComponent.onAlerttableDeleteMultiRecords()', this.selectedAlerts);
  }

  /**
  * This API serves html template.
  *  It will archive selected alerts
  */
  onAlerttableArchiveMultiRecords($event) {
    console.log('AlerttableComponent.onAlerttableArchiveMultiRecords()', this.selectedAlerts);
  }

}
