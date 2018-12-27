import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { Utility } from '@app/common/Utility';
import { IMonitoringFwlogPolicy, IMonitoringFlowExportPolicy, MonitoringFwlogPolicy, MonitoringFlowExportPolicy } from '@sdk/v1/models/generated/monitoring';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { MessageService } from 'primeng/primeng';
import { BaseComponent } from '@app/components/base/base.component';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { Subscription } from 'rxjs';

@Component({
  selector: 'app-telemetry',
  templateUrl: './telemetry.component.html',
  styleUrls: ['./telemetry.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class TelemetryComponent extends BaseComponent implements OnInit {
  fwlogPolicies: ReadonlyArray<IMonitoringFwlogPolicy> = [];
  flowExportPolicies: ReadonlyArray<IMonitoringFlowExportPolicy> = [];

  fwlogPoliciesEventUtility: HttpEventUtility<MonitoringFwlogPolicy>;
  flowExportPoliciesEventUtility: HttpEventUtility<MonitoringFlowExportPolicy>;

  subscriptions: Subscription[] = [];

  constructor(
    protected controllerService: ControllerService,
    protected monitoringService: MonitoringService,
    protected messageService: MessageService
  ) {
    super(controllerService, messageService);
  }

  ngOnInit() {
    // Setting the toolbar of the app
    this.controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Telemetry Policies', url: Utility.getBaseUIUrl() + 'monitoring/telemetry' }]
    });
    this.getFwlogPolicies();
    this.getFlowExportPolicies();
  }

  getFwlogPolicies() {
    this.fwlogPoliciesEventUtility = new HttpEventUtility<MonitoringFwlogPolicy>(MonitoringFwlogPolicy);
    this.fwlogPolicies = this.fwlogPoliciesEventUtility.array;
    const subscription = this.monitoringService.WatchFwlogPolicy().subscribe(
      (response) => {
        this.fwlogPoliciesEventUtility.processEvents(response);
      },
      this.restErrorHandler('Failed to get Firewall Log Policies')
    );
    this.subscriptions.push(subscription);
  }

  getFlowExportPolicies() {
    this.flowExportPoliciesEventUtility = new HttpEventUtility<MonitoringFlowExportPolicy>(MonitoringFlowExportPolicy);
    this.flowExportPolicies = this.flowExportPoliciesEventUtility.array;
    const subscription = this.monitoringService.WatchFlowExportPolicy().subscribe(
      (response) => {
        this.flowExportPoliciesEventUtility.processEvents(response);
      },
      this.restErrorHandler('Failed to get Flow Export Policies')
    );
    this.subscriptions.push(subscription);
  }

}
