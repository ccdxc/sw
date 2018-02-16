import { Component, OnInit } from '@angular/core';
import { ControllerService } from '../../services/controller.service';
import {Eventtypes} from '../../enum/eventtypes.enum';
import {Logintypes} from '../../enum/logintypes.enum';
import { Utility } from '../../common/Utility';
import { CommonComponent } from '../../common.component';

declare var google: any;


@Component({
  selector: 'app-base',
  templateUrl: './base.component.html',
  styleUrls: ['./base.component.scss']
})
export class BaseComponent extends CommonComponent implements OnInit {
  protected static googleLoaded: any;
  private _interval: any;

  constructor(protected _controllerService: ControllerService) {
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



  protected loadGoogleChart() {
    if (!BaseComponent.googleLoaded) {
      google.charts.load('current', {'packages': ['corechart']});
      this._controllerService.publish(Eventtypes.GOOGLE_CHART_LOADING, {'googleChartLoading': 'true'});
      this._interval = setInterval(() => {
        if (google.visualization && google.visualization.ChartWrapper ) {
          this._googleChartLoaded();
          window.clearInterval(this._interval);
        }
      }, 1000);
    }
    google.charts.setOnLoadCallback(() => {
        window.clearInterval(this._interval);
        this._googleChartLoaded();
    });
  }

  _googleChartLoaded() {
      BaseComponent.googleLoaded = true;
      this.drawGraph();
      this._controllerService.publish(Eventtypes.GOOGLE_CHART_LOADED, {'googleChartLoaded': 'true'});
  }
  // */

  drawGraph() {
      this.log('GoogleChart loaded !!!! google.visualization ' + google.visualization);
  }



  createDataTable(array: any[]): any {
      return google.visualization.arrayToDataTable(array);
  }

  createChartWrapper(chartOptions, chartType, chartData, elementId): any {
    return new google.visualization.ChartWrapper({
             chartType: chartType,
             dataTable: chartData ,
             options: chartOptions || {},
             containerId: elementId
           });
  }

  addEventHandler(chartWrapper, eventName, handlerFunction) {
    this.debug('googlechart:' + eventName);
    google.visualization.events.addListener(chartWrapper, eventName, handlerFunction);
  }

  formatDate(obj: any): Date {
      return new Date(obj);
  }

  protected _publishAJAXStart() {
    this.errorMessage = '';
    Utility.getInstance().publishAJAXStart( {'ajax': 'start', 'name': 'cv-AJAX'});
  }

  protected _publishAJAXEnd() {
    this.errorMessage = '';
    this.successMessage = '';
    this.autosaveMessage.message = '';
    Utility.getInstance().publishAJAXEnd( {'ajax': 'end', 'name': 'cv-AJAX'});
  }

  /**
   * This API detects whether an object is empty  -- {}
   */
  protected isObjectEmpty(obj): boolean {
    let count = 0;
    if (obj instanceof Date) {
      return false;
    }
    if ( typeof(obj) === 'boolean') {
      return false;
    }
    if ( typeof(obj) === 'string') {
      return false;
    }
    /*if (obj!=null){
      return false;
    }*/
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
    if (typeof parentObj !== 'object') {return; }
    if (!parentObj) {return; }
    for (const prop in parentObj) {
        if ( this.isObjectEmpty(parentObj[prop])) {
            delete parentObj[prop];
        }else {
            this.trimJSON(parentObj[prop]);
        }
    }
}

/**
 * This API serves html template. It allows user to click off error message.
 */
onErrorMessageClick() {
    if (this.errorMessage) {
        this.errorMessage = null;
    }
}

showHideSuccessMessage(message: string , milliSeconds: number = 1500) {
    this.successMessage = message;
    if (this.successMessage) {
        setTimeout(() => {
         this.successMessage = null;
        }, milliSeconds);
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


}
