import { Component, OnInit, ViewEncapsulation, OnDestroy } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { BaseComponent } from '@app/components/base/base.component';
import { IMonitoringAlertDestination, IMonitoringAlertPolicy, IApiStatus } from '@sdk/v1/models/generated/monitoring';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Subscription } from 'rxjs/Subscription';

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

  eventPoliciesEventUtility: HttpEventUtility;
  metricPoliciesEventUtility: HttpEventUtility;
  objectPoliciesEventUtility: HttpEventUtility;
  destinationsEventUtility: HttpEventUtility;

  subscriptions: Subscription[] = [];

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    this.getAlertPolicies();
    this.getDestinations();
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

  getAlertPolicies() {
    this.eventPoliciesEventUtility = new HttpEventUtility(
      (policy) => {
        return policy.spec.resource === 'Event';
      }
    );
    this.metricPoliciesEventUtility = new HttpEventUtility(
      (policy) => {
        return policy.spec.resource === 'EndpointMetrics';
      }
    );
    this.objectPoliciesEventUtility = new HttpEventUtility(
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
      (error) => {
        // TODO: Error handling
        if (error.body instanceof Error) {
          console.log('Monitoring service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.log('Monitoring service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
        }
      }
    );
    this.subscriptions.push(subscription);
  }

  getDestinations() {
    this.destinationsEventUtility = new HttpEventUtility();
    this.destinations = this.destinationsEventUtility.array;
    const subscription = this._monitoringService.WatchAlertDestination().subscribe(
      (response) => {
        const body: any = response.body;
        this.destinationsEventUtility.processEvents(body);
      },
      (error) => {
        // TODO: Error handling
        if (error.body instanceof Error) {
          console.log('Monitoring service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.log('Monitoring service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
        }
      }
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
