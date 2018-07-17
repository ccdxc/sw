import { Component, OnInit, ViewEncapsulation, OnDestroy } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { BaseComponent } from '@app/components/base/base.component';
import { IMonitoringAlertPolicyList, IMonitoringAlertDestinationList, IMonitoringAlertPolicy, IMonitoringAlertDestination  } from '@sdk/v1/models/generated/monitoring';
import { ApiStatus } from '@sdk/v1/models/generated/monitoring';

@Component({
  selector: 'app-alertpolicies',
  templateUrl: './alertpolicies.component.html',
  styleUrls: ['./alertpolicies.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class AlertpoliciesComponent extends BaseComponent implements OnInit, OnDestroy {
  alertPolices: any;
  eventPolicies: IMonitoringAlertPolicy[] = [];
  metricPolicies: IMonitoringAlertPolicy[] = [];
  objectPolicies: IMonitoringAlertPolicy[] = [];
  destinations: IMonitoringAlertDestination[] = [];

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    this.refresh();
    this._controllerService.setToolbarData({
      buttons: [
      ],
      breadcrumb: [
        { label: 'Alerts & Events', url: '/#/monitoring/alertsevents' },
        { label: 'Alert Policies', url: '/#/monitoring/alertsevents/alertpolicies' }
      ]
    });
  }

  /**
  * Overide super's API
  * It will return this Component name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  refresh() {
    this.getAlertPolicies();
    this.getDestinations();
  }

  getAlertPolicies() {
    this._monitoringService.ListAlertPolicy().subscribe(response => {
      const status = response.statusCode;
      if (status === 200) {
        const body: IMonitoringAlertPolicyList = response.body as IMonitoringAlertPolicyList;
        const eventPolicies = [];
        const metricPolicies = [];
        const objectPolicies = [];
        if (body.Items != null) {
          body.Items.forEach((policy) => {
            if (policy.spec.resource === 'Event') {
              eventPolicies.push(policy);
            } else if (policy.spec.resource === 'EndpointMetrics') {
              metricPolicies.push(policy);
            } else {
              objectPolicies.push(policy);
            }
          });
        }
        this.eventPolicies = eventPolicies;
        this.metricPolicies = metricPolicies;
        this.objectPolicies = objectPolicies;
      } else {
        const body: ApiStatus = response.body as ApiStatus;
        console.log(body);
      }
    });
  }

  getDestinations() {
    this._monitoringService.ListAlertDestination().subscribe(response => {
      const status = response.statusCode;
      if (status === 200) {
        const body: IMonitoringAlertDestinationList = response.body as IMonitoringAlertDestinationList;
        if (body.Items != null) {
          this.destinations = body.Items;
        }
      } else {
        const body: ApiStatus = response.body as ApiStatus;
        console.log(body);
      }
    });
  }

  ngOnDestroy() {
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': 'AlertpoliciesComponent', 'state':
        Eventtypes.COMPONENT_DESTROY
    });
  }


}
