import { OnInit } from '@angular/core';
import { AbstractControl, FormArray } from '@angular/forms';
import { Utility } from '@app/common/Utility';
import { LogService } from '@app/services/logging/log.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { SortEvent } from 'primeng/components/common/api';
import { ControllerService } from '../../services/controller.service';
import { TableUtility } from '../shared/tableviewedit/tableutility';
import { Subscription } from 'rxjs';
// declare var google: any;

/**
 * Basic component that all components should extend from
 */
export class BaseComponent implements OnInit {
  // protected static googleLoaded: any;
  private _interval: any;
  protected logger: LogService;
  protected subscriptions: Subscription[] = [];

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService = null) {
    this.logger = Utility.getInstance().getLogService();
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

  routeToHomepage() {
    if (this.uiconfigsService != null) {
      this.uiconfigsService.navigateToHomepage();
    } else {
      console.error('UIConfig service is null');
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
    let res = '';
    Object.keys(item).forEach(key => {
      res = `${res}"${key}":"${item[key]}"\n`;
    });
    return res;
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


  displayColumn(data, col, hasUiHints: boolean = true): any {
    return TableUtility.displayColumn(data, col, hasUiHints);
  }

  controlAsFormArray(control: AbstractControl): FormArray {
    return control as FormArray;
  }

  debug(msg: string, ...optionalParams: any[]) {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.debug(msg, caller, optionalParams);
  }

  info(msg: string, ...optionalParams: any[]) {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.info(msg, caller, optionalParams);
  }

  warn(msg: string, ...optionalParams: any[]) {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.warn(msg, caller, optionalParams);
  }

  error(msg: string, ...optionalParams: any[]) {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.error(msg, caller, optionalParams);
  }

  fatal(msg: string, ...optionalParams: any[]) {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.fatal(msg, caller, optionalParams);
  }

  log(msg: string, ...optionalParams: any[]) {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.log(msg, caller, optionalParams);
  }

  clear(): void {
    if (!this.logger) {
      console.error('common.component.ts logger is null');
      return;
    }
    this.logger.clear();
  }

  unsubscribeAll() {

    this.subscriptions.forEach( (sub) => {
      sub.unsubscribe();
    });
  }

  isFieldEmpty(field: AbstractControl): boolean {
    return Utility.isEmpty(field.value);
  }

}
