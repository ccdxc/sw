import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { BaseComponent } from '@app/components/base/base.component';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IMonitoringFlowExportPolicy, MonitoringFlowExportPolicy } from '@sdk/v1/models/generated/monitoring';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Subscription } from 'rxjs';

@Component({
  selector: 'app-flowexport',
  templateUrl: './flowexport.component.html',
  styleUrls: ['./flowexport.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class FlowexportComponent extends BaseComponent implements OnInit {
  constructor(
    protected controllerService: ControllerService,
    protected monitoringService: MonitoringService,
  ) {
    super(controllerService);
  }
  flowExportPolicies: ReadonlyArray<IMonitoringFlowExportPolicy> = [];
  flowExportPoliciesEventUtility: HttpEventUtility<MonitoringFlowExportPolicy>;

  subscriptions: Subscription[] = [];

  bodyIcon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/monitoring/ico-export-policy-black.svg'
  };

  ngOnInit() {
    // Setting the toolbar of the app
    this.controllerService.setToolbarData({
      buttons: [],
      breadcrumb: []
    });
    this.getFlowExportPolicies();
  }

  getFlowExportPolicies() {
    this.flowExportPoliciesEventUtility = new HttpEventUtility<MonitoringFlowExportPolicy>(MonitoringFlowExportPolicy);
    this.flowExportPolicies = this.flowExportPoliciesEventUtility.array;
    const subscription = this.monitoringService.WatchFlowExportPolicy().subscribe(
      (response) => {
        this.flowExportPoliciesEventUtility.processEvents(response);
      },
      this.controllerService.webSocketErrorHandler('Failed to get Policies')
    );
    this.subscriptions.push(subscription);
  }
}
