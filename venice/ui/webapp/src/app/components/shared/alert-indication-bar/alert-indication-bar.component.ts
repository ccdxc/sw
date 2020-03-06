import { Component, Input, OnChanges, EventEmitter, Output } from '@angular/core';
import { MonitoringAlert } from '@sdk/v1/models/generated/monitoring';

@Component({
  selector: 'app-alert-indication-bar',
  templateUrl: './alert-indication-bar.component.html',
  styleUrls: ['./alert-indication-bar.component.scss'],
})
export class AlertIndicationBarComponent implements OnChanges {
  @Input() data: ReadonlyArray<MonitoringAlert> = [];
  @Output() alertType: EventEmitter<{ event, alertType: string }> = new EventEmitter();

  alertNumber: AlertNumber = {
    total: 0,
    critical: 0,
    warn: 0,
    info: 0
  };
  menuIndicatorCSS = {};
  alerts: any[] = [];
  currentAlertFilterType: string = 'total';
  constructor() { }

  ngOnChanges() {
    if (this.data && Array.isArray(this.data)) {
      this.alerts = this.data;
      this.computeAlertNumbers();
    }
  }

  computeAlertNumbers() {
    this.clearAlertNumber();
    this.alertNumber.total = this.alerts.length;
    this.alerts.forEach(alert => {
      this.alertNumber[alert.status.severity] += 1;
      if (this.alertNumber[alert.status.severity] > 9999) {
        this.alertNumber[alert.status.severity] = Number((this.alertNumber[alert.status.severity]).toString().substring(0, 4));
      }
    });

  }
  /**
   * This API serves html template.
   * It will filter alerts displayed in list
   */
  onAlertNumber(event, alertType: string) {
    this.alertType.emit({ event, alertType });
  }

  clearAlertNumber() {
    this.alertNumber = {
      total: 0,
      critical: 0,
      warn: 0,
      info: 0
    };
  }

}

interface AlertNumber {
  total: number;
  critical: number;
  warn: number;
  info: number;
}

