import { Component, OnInit, ViewEncapsulation, ViewChild } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { AlerttableService } from '@app/services/alerttable.service';
import { BaseComponent } from '@app/components/base/base.component';
import { Utility } from '@app/common/Utility';
import { Table } from 'primeng/table';

@Component({
  selector: 'app-alertsevents',
  templateUrl: './alertsevents.component.html',
  styleUrls: ['./alertsevents.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class AlertseventsComponent extends BaseComponent implements OnInit {
  @ViewChild('alerttable') alertTurboTable: Table;

  cols: any[] = [
    { field: 'date', header: 'Date', class: 'alertsevents-column-date', sortable: true },
    { field: 'name', header: 'Name', class: 'alertsevents-column-name', sortable: false },
    { field: 'severity', header: 'Severity', class: 'alertsevents-column-severity', sortable: false },
    { field: 'message', header: 'Message', class: 'alertsevents-column-message', sortable: false },
    { field: 'policy', header: 'Policy', class: 'alertsevents-column-policy', sortable: false },
  ];
  alerts: any;
  selectedAlerts: any = [];

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

  ngOnInit() {
    if (!this._controllerService.isUserLogin()) {
      this._controllerService.publish(Eventtypes.NOT_YET_LOGIN, {});
    } else {
      this._controllerService.setToolbarData({
        buttons: [
          {
            cssClass: 'global-button-primary alertsevents-button',
            text: 'ALERT POLICIES',
            callback: () => { this._controllerService.navigate(['/monitoring', 'alertsevents', 'alertpolicies']); }
          }
        ],
        breadcrumb: [{ label: 'Alerts & Events', url: '/#/monitoring/alertsevents' }]
      });
      this.getAlerts();
    }
  }

  getClassName(): string {
    return this.constructor.name;
  }

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
    return 'global-alert-' + record.severity;
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
