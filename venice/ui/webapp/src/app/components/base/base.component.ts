import { Component, OnInit } from '@angular/core';
import { FormArray, FormGroup } from '@angular/forms';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MessageService, SortEvent } from 'primeng/components/common/api';
import { CommonComponent } from '../../common.component';
import { ControllerService } from '../../services/controller.service';
import { Utility } from '@app/common/Utility';
// declare var google: any;

/**
 * Basic component that all components should extend from
 */
@Component({
  selector: 'app-base',
  templateUrl: './base.component.html',
  styleUrls: ['./base.component.scss']
})
export class BaseComponent extends CommonComponent implements OnInit {
  // protected static googleLoaded: any;
  private _interval: any;

  constructor(protected _controllerService: ControllerService,
    protected messageService: MessageService,
    protected uiconfigsService: UIConfigsService = null) {
    super();
  }

  ngOnInit() {
  }

  /**
   * Overide super's API
   * It will return this Component name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  invokeSuccessToaster(summary, detail) {
    this._controllerService.invokeSuccessToaster(summary, detail);
  }

  invokeRESTErrorToaster(summary, error) {
    this._controllerService.invokeRESTErrorToaster(summary, error);
  }

  restErrorHandler(summary) {
    return this._controllerService.restErrorHandler(summary);
  }

  routeToHomepage() {
    if (this.uiconfigsService != null) {
      this.uiconfigsService.navigateToHomepage();
    }
  }


  /**
   * Following code is for using google charts
   * Currently we are not using it, but leaving this
   * here for future use.
   */

  // protected loadGoogleChart() {
  //   if (!BaseComponent.googleLoaded) {
  //     google.charts.load('current', { 'packages': ['corechart'] });
  //     this._controllerService.publish(Eventtypes.GOOGLE_CHART_LOADING, { 'googleChartLoading': 'true' });
  //     this._interval = setInterval(() => {
  //       if (google.visualization && google.visualization.ChartWrapper) {
  //         this._googleChartLoaded();
  //         window.clearInterval(this._interval);
  //       }
  //     }, 1000);
  //   }
  //   google.charts.setOnLoadCallback(() => {
  //     window.clearInterval(this._interval);
  //     this._googleChartLoaded();
  //   });
  // }

  // _googleChartLoaded() {
  //   BaseComponent.googleLoaded = true;
  //   this.drawGraph();
  //   this._controllerService.publish(Eventtypes.GOOGLE_CHART_LOADED, { 'googleChartLoaded': 'true' });
  // }

  // drawGraph() {
  //   this.log('GoogleChart loaded !!!! google.visualization ' + google.visualization);
  // }

  // createDataTable(array: any[]): any {
  //   return google.visualization.arrayToDataTable(array);
  // }

  // createChartWrapper(chartOptions, chartType, chartData, elementId): any {
  //   return new google.visualization.ChartWrapper({
  //     chartType: chartType,
  //     dataTable: chartData,
  //     options: chartOptions || {},
  //     containerId: elementId
  //   });
  // }

  // addEventHandler(chartWrapper, eventName, handlerFunction) {
  //   this.debug('googlechart:' + eventName);
  //   google.visualization.events.addListener(chartWrapper, eventName, handlerFunction);
  // }

  formatDate(obj: any): Date {
    return new Date(obj);
  }

  /**
   * This API detects whether an object is empty  -- {}
   */
  protected isObjectEmpty(obj): boolean {
    let count = 0;
    if (obj instanceof Date) {
      return false;
    }
    if (typeof (obj) === 'boolean') {
      return false;
    }
    if (typeof (obj) === 'string') {
      return false;
    }
    for (const attr in obj) {
      if (obj.hasOwnProperty(attr)) {
        count += 1;
      }
    }
    return (count === 0);
  }

  /**
   * This API traverse JSON object to trim any empty sub-treee
   */
  trimJSON(parentObj) {
    if (typeof parentObj !== 'object') { return; }
    if (!parentObj) { return; }
    for (const prop in parentObj) {
      if (this.isObjectEmpty(parentObj[prop])) {
        delete parentObj[prop];
      } else {
        this.trimJSON(parentObj[prop]);
      }
    }
  }

  /**
   * This is a helper function
   */
  protected removeObjectByKeyFromList(value: string, list: any, oneOnly: boolean = true, key?: any): any {
    if (!key) {
      key = 'value';
    }
    const len = list.length;
    for (let i = len - 1; i >= 0; i--) {
      const listObj = list[i];
      if (listObj[key] === value) {
        list.splice(i, 1);
        if (oneOnly) {
          break;
        }
      }
    }
    return list;
  }

  /**
   * This is a helper function
   */
  protected getSelectedValueObject(value: string, list: any, key?: any): any {
    if (!key) {
      key = 'value';
    }
    for (let i = 0; list && i < list.length; i++) {
      const listObj = list[i];
      if (listObj[key] === value) {
        return listObj;
      }
    }
    return null;
  }

  displayLabels(item): string {
    return JSON.stringify(item);
  }

  /**
   * Default table column sort API.
   * @param event
   */
  customSort(event: SortEvent) {
    event.data.sort((data1, data2) => {
      const value1 = data1[event.field];
      const value2 = data2[event.field];
      let result = null;

      if (value1 == null && value2 != null) {
        result = -1;
      } else if (value1 != null && value2 == null) {
        result = 1;
      } else if (value1 == null && value2 == null) {
        result = 0;
      } else if (typeof value1 === 'string' && typeof value2 === 'string') {
        result = value1.localeCompare(value2);
      } else {
        result = (value1 < value2) ? -1 : (value1 > value2) ? 1 : 0;
      }

      return (event.order * result);
    });
  }

  getObjectKeys(obj): string[] {
    return (obj) ? Object.keys(obj) : [];
  }

  stringify(obj): string {
    return JSON.stringify(obj, null, 1);
  }

  getAllFormgroupErrors(form: FormGroup | FormArray): { [key: string]: any; } | null {
    let hasError = false;
    const result = Object.keys(form.controls).reduce((acc, key) => {
      // for debug: console.log('basecomponent.getAllFormgroupErrors()', acc , key);
      const control = form.get(key);
      let errors = null;
      if (control instanceof FormGroup || control instanceof FormArray) {
        errors = this.getAllFormgroupErrors(control);
      } else {
        errors = control.invalid;
      }
      if (errors) {
        acc[key] = errors;
        hasError = true;
      }
      return acc;
    }, {} as { [key: string]: any; });
    return hasError ? result : null;
  }

  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

}
