import { Component, OnInit, ViewEncapsulation, OnDestroy } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { BaseComponent } from '@app/components/base/base.component';
import { IMonitoringAlertDestination, IMonitoringAlertPolicy, IApiStatus, MonitoringAlertPolicy, MonitoringAlertDestination } from '@sdk/v1/models/generated/monitoring';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Subscription } from 'rxjs';
import { MessageService } from 'primeng/primeng';
import { Utility } from '@app/common/Utility';

@Component({
  selector: 'app-alertpolicies',
  templateUrl: './alertpolicies.component.html',
  styleUrls: ['./alertpolicies.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class AlertpoliciesComponent extends BaseComponent implements OnInit, OnDestroy {
  eventPolicies: ReadonlyArray<IMonitoringAlertPolicy> = [];
  metricPolicies: ReadonlyArray<IMonitoringAlertPolicy> = [];
  objectPolicies: ReadonlyArray<IMonitoringAlertPolicy> = [];
  destinations: ReadonlyArray<IMonitoringAlertDestination> = [];

  eventPoliciesEventUtility: HttpEventUtility<MonitoringAlertPolicy>;
  metricPoliciesEventUtility: HttpEventUtility<any>;
  objectPoliciesEventUtility: HttpEventUtility<any>;
  destinationsEventUtility: HttpEventUtility<MonitoringAlertDestination>;

  subscriptions: Subscription[] = [];

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
    protected messageService: MessageService
  ) {
    super(_controllerService, messageService);
  }

  ngOnInit() {
    this.getAlertPolicies();
    this.getDestinations();
    this._controllerService.setToolbarData({
      buttons: [
      ],
      breadcrumb: [
        { label: 'Alerts & Events', url: Utility.getBaseUIUrl() + 'monitoring/alertsevents' },
        { label: 'Alert Policies', url: Utility.getBaseUIUrl() + 'monitoring/alertsevents/alertpolicies' }
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

  getAlertPolicies() {
    this.eventPoliciesEventUtility = new HttpEventUtility<MonitoringAlertPolicy>(MonitoringAlertPolicy, false,
      (policy) => {
        return policy.spec.resource === 'Event';
      }
    );
    this.metricPoliciesEventUtility = new HttpEventUtility<any>(null, false,
      (policy) => {
        return policy.spec.resource === 'EndpointMetrics';
      }
    );
    this.objectPoliciesEventUtility = new HttpEventUtility<any>(null, false,
      (policy) => {
        return policy.spec.resource !== 'EndpointMetrics' &&
          policy.spec.resource !== 'Event';
      }
    );
    this.eventPolicies = this.eventPoliciesEventUtility.array;
    this.metricPolicies = this.metricPoliciesEventUtility.array;
    this.objectPolicies = this.objectPoliciesEventUtility.array;
    const subscription = this._monitoringService.WatchAlertPolicy().subscribe(
      (response) => {
        const body: any = response.body;
        this.eventPoliciesEventUtility.processEvents(body);
        this.metricPoliciesEventUtility.processEvents(body);
        this.objectPoliciesEventUtility.processEvents(body);
      },
      this.restErrorHandler('Failed to get Alert Policies')
    );
    this.subscriptions.push(subscription);
  }

  getDestinations() {
    this.destinationsEventUtility = new HttpEventUtility<MonitoringAlertDestination>(MonitoringAlertDestination);
    this.destinations = this.destinationsEventUtility.array;
    const subscription = this._monitoringService.WatchAlertDestination().subscribe(
      (response) => {
        const body: any = response.body;
        this.destinationsEventUtility.processEvents(body);
      },
      this.restErrorHandler('Failed to get Alert Destinations')
    );
    this.subscriptions.push(subscription);
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': 'AlertpoliciesComponent', 'state':
        Eventtypes.COMPONENT_DESTROY
    });
  }


}
