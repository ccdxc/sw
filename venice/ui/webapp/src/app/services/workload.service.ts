import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { BehaviorSubject, Observable, Subscription } from 'rxjs';

import { environment } from '../../environments/environment';
import { Utility } from '../common/Utility';
import { Eventtypes } from '../enum/eventtypes.enum';
import { AbstractService } from './abstract.service';



@Injectable()
export class WorkloadService extends AbstractService {
  /* Static Data */
  svgMap: any = {
    totalworkloads: 'workloads-total',
    newworkloads: 'workloads-new',
    unprotectedworkloads: 'workloads-unprotected',
    workloadalerts: 'workloads-alert',
  };
  staticData: any = {
    totalworkloads: {
      title: 'Total Workloads',
      icon: {
        margin: {
          top: '6px',
          left: '10px',
          right: '5px'
        },
        svgIcon: this.svgMap['totalworkloads']
      },
      background_img: {
        url: 'assets/images/icons/workload/icon-workloads-total.svg'
      }
    },
    newworkloads: {
      title: 'New Workloads',
      icon: {
        margin: {
          top: '5px',
          left: '10px',
          right: '10px'
        },
        svgIcon: this.svgMap['newworkloads']
      },
      background_img: {
        url: 'assets/images/icons/workload/icon-workloads-new.svg'
      }
    },
    unprotectedworkloads: {
      title: 'Unprotected Workloads',
      icon: {
        margin: {
          top: '6px',
          left: '10px',
          right: '5px'
        },
        svgIcon: this.svgMap['unprotectedworkloads']
      },
      background_img: {
        url: 'assets/images/icons/workload/icon-workloads-unprotected.svg'
      }
    },
    workloadalerts: {
      title: 'Workload Alerts',
      icon: {
        margin: {
          top: '2px',
          left: '10px',
          right: '5px'
        },
        svgIcon: this.svgMap['workloadalerts']
      },
      background_img: {
        url: 'assets/images/icons/workload/icon-workload-alerts.svg'
      }
    }
  };

  /* Holders for fetched data */
  rawEndpointData: any;
  alertCount = 0;
  dsbdworkload: any = {
    veniceEndpointCount: 0,
    veniceNewEndpointCount: 0,
    endpointList: [],
    endpointPercent: [],
    endpointPercentCompliment: [],
  };
  workloads: Array<any> = [];
  widgetData: any;
  totalworkloads: any = {
    data: null,
    label: null,
    style: null,
  };
  newworkloads: any = {
    data: null,
    label: null,
    style: null,
  };
  unprotectedworkloads: any = {
    data: null,
    label: null,
    style: null,
  };
  workloadalerts: any = {
    data: null,
    label: null,
    style: null,
  };

  dataFetched = false;

  private dsbdworkloadHandler = new BehaviorSubject<any>(this.dsbdworkload);

  private workloadItemsHandler = new BehaviorSubject<any>(this.workloads);
  private totalworkloadsHandler = new BehaviorSubject<any>(this.totalworkloads);
  private unprotectedworkloadsHandler = new BehaviorSubject<any>(this.unprotectedworkloads);
  private newworkloadsHandler = new BehaviorSubject<any>(this.newworkloads);
  private workloadalertsHandler = new BehaviorSubject<any>(this.workloadalerts);

  constructor(
    private _http: HttpClient,
    protected _controllerService: ControllerService) {
    super();
  }

  /**
   * Override super
  */
  protected callServer(url: string, payload: any) {
    return this.invokeAJAXPostCall(url, payload,
      this._http, { 'ajax': 'start', 'name': 'WorkloadService-ajax', 'url': url });
  }

  protected computeStyleAndArrow(id, graphData) {
    const styleOptions = ['positive', 'negative'];
    const arrowOptions = ['up', 'down'];
    let styleIndex = 0;
    let arrowIndex;
    if (graphData.y[graphData.y.length - 1] < graphData.y[graphData.y.length - 2]) {
      styleIndex = 1;
      arrowIndex = 1;
    }
    if (id === 'unprotectedworkloads' || id === 'workloadalerts') {
      // Lower numbers are good for these ids, so we invert the color
      styleIndex = styleIndex + 1;
    }
    return { style: styleOptions[styleIndex % 2], arrow: arrowOptions[arrowIndex % 2] };
  }


  private populateWorkloadwidgetData() {
    // compute styling
    this.totalworkloads = this.widgetData.totalworkloads;
    this.newworkloads = this.widgetData.newworkloads;
    this.unprotectedworkloads = this.widgetData.unprotectedworkloads;
    // We dont use the generated workload alerts label, instead we use
    // it based off the fetch workload items call
    this.workloadalerts.data = this.widgetData.workloadalerts.data;
    this.workloadalerts.data.y[this.workloadalerts.data.y.length - 1] = this.alertCount;
    this.totalworkloads.label = this.workloads.length;
    this.totalworkloads.data.y[this.totalworkloads.data.y - 1] = this.workloads.length;
    // Compute styling
    const ids = ['totalworkloads', 'newworkloads', 'unprotectedworkloads', 'workloadalerts'];
    ids.forEach(id => {
      const settings = this.computeStyleAndArrow(id, this[id].data);
      this[id].style = settings.style;
      this[id].arrow = settings.arrow;
      const handlerId = id.concat('Handler');
      this[handlerId].next(this[id]);
    });
  }

  private populateWorkloads() {
    const workloads = [];
    this.alertCount = 0;

    const data = this.rawEndpointData.Items;

    const formatLabels = (labels) => {
      let ret = '';
      if (labels == null) {
        return ret;
      }
      for (const key of Object.keys(labels)) {
        ret = ret.concat(labels[key]);
        ret = ret.concat(',');
      }
      // removing extra comma
      return ret.slice(0, -1);
    };

    const formatSecurityGroups = (securityGroups) => {
      let ret = '';
      securityGroups.forEach(element => {
        ret = ret.concat(element);
        ret = ret.concat(',');
      });
      // removing extra comma
      return ret.slice(0, -1);
    };
    const getRandomState = () => {
      const options = ['neutral', 'changed', 'deleted', 'alert'];
      return options[Utility.getRandomInt(0, 3)];
    };

    data.forEach(elem => {
      const workload = {
        name: elem.meta.name,
        labels: formatLabels(elem.meta.labels),
        securityGroups: formatSecurityGroups(elem.status.SecurityGroups),
        // Fake data
        orchestration: 'ORCH-NAME',
        loadBalancer: 'LB-NAME',
        appId: 'APP-ID Name',
        state: getRandomState()
      };
      if (workload.state === 'alert') {
        this.alertCount++;
      }
      workloads.push(workload);
    });
    this.workloads = workloads;
    // update workload alert count and total workloads count
    this.workloadalerts.label = this.alertCount;
    this.totalworkloads.label = this.workloads.length;
    // keeping fake data consistent with real data
    if (this.workloadalerts.data != null) {
      this.workloadalerts.data.y[this.workloadalerts.data.y - 1] = this.alertCount;
    }
    if (this.totalworkloads.data != null) {
      this.totalworkloads.data.y[this.totalworkloads.data.y - 1] = this.workloads.length;
    }
    this.workloadItemsHandler.next(this.workloads);
  }

  private fetchData(useRealData, url, onSuccess, onError) {
    this.invokeAJAXGetCall(url,
      this._http, { 'ajax': 'start', 'name': 'WorkloadService-ajax', 'url': url }, useRealData)
      .subscribe(
        data => {
          const isRESTPassed = Utility.isRESTSuccess(data);
          if (isRESTPassed) {
            onSuccess(data);
          } else {
            onError();
          }
        },
        err => {
          this.error(err);
        }
      );
  }

  private fetchWorkloadData(useRealData): void {
    const url = this.getItemURL();
    const onSuccess = (data) => {
      this._controllerService.publish(Eventtypes.AJAX_END, { 'ajax': 'end', 'name': 'WORKLOAD_GET_ITEMS' });
      this.rawEndpointData = data;
      this.populateWorkloads();
    };
    const onError = () => {
      this._controllerService.publish(Eventtypes.AJAX_END, { 'ajax': 'end', 'name': 'WORKLOAD_GET_ITEMS' });
    };
    this.fetchData(useRealData, url, onSuccess, onError);
  }

  fetchWidgetData(useRealData): void {
    // Since endpoint isn't ready yet, this function will always be using
    // fake data.
    const url = this.getWidgetURL();
    const onSuccess = (data) => {
      this._controllerService.publish(Eventtypes.AJAX_END, { 'ajax': 'end', 'name': 'WORKLOAD_GET_WIDGET_DATA' });
      this.widgetData = data;
      this.populateWorkloadwidgetData();
    };
    const onError = () => {
      this._controllerService.publish(Eventtypes.AJAX_END, { 'ajax': 'end', 'name': 'WORKLOAD_GET_WIDGET_DATA' });
    };
    this.fetchData(useRealData, url, onSuccess, onError);
  }

  fetchDsbdWorkloadData(useRealData): void {
    const url = this.getDashboardWidgetURL();
    const onSuccess = (data) => {
      this._controllerService.publish(Eventtypes.AJAX_END, { 'ajax': 'end', 'name': 'WORKLOAD_GET_DSBDWIDGET_DATA' });
      this.dsbdworkload = data;
      this.dsbdworkloadHandler.next(data);
    };
    const onError = () => {
      this._controllerService.publish(Eventtypes.AJAX_END, { 'ajax': 'end', 'name': 'WORKLOAD_GET_DSBDWIDGET_DATA' });
    };
    this.fetchData(useRealData, url, onSuccess, onError);

  }

  /**
   * Override super
   * Get the service class-name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  public getItemURL(): string {
    if (!environment.production) {
      if (!this.isToMockData()) {
        return '/endpoints';
      }
    }
    return Utility.getRESTAPIServerAndPort() + environment.version_api_string + this.buildURLHelper(environment.venice_endpoints);
  }

  public getWidgetURL(): string {
    return 'workloadwidget';
  }

  public getDashboardWidgetURL(): string {
    return 'dsbdworkloadwidget';
  }

  protected pollingHasObservers(): boolean {
    return !(this.workloadItemsHandler.observers.length === 0 &&
      this.totalworkloadsHandler.observers.length === 0 &&
      this.unprotectedworkloadsHandler.observers.length === 0 &&
      this.newworkloadsHandler.observers.length === 0 &&
      this.workloadalertsHandler.observers.length === 0 &&
      this.dsbdworkloadHandler.observers.length === 0);
  }

  protected pollingFetchData(useRealData) {
    this.fetchWorkloadData(useRealData);
    this.fetchWidgetData(false);
    this.fetchDsbdWorkloadData(false);
  }

  public getDsbdWidgetData(): Observable<any> {
    // TODO: Switch to true once real data is ready
    this.initiatePolling(false);
    return this.dsbdworkloadHandler;
  }

  public getItems(): Observable<any> {
    this.initiatePolling(this._controllerService.useRealData);
    return this.workloadItemsHandler;
  }

  public getWidgetDataById(id): Observable<any> {
    const idMap = {
      totalworkloads: this.totalworkloadsHandler,
      newworkloads: this.newworkloadsHandler,
      unprotectedworkloads: this.unprotectedworkloadsHandler,
      workloadalerts: this.workloadalertsHandler,
    };
    this.initiatePolling(this._controllerService.useRealData);
    return idMap[id];
  }

  public isToMockData(): boolean {
    return true;
  }

  public getStaticWidgetDataById(id: string) {
    return this.staticData[id];
  }

}

