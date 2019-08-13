import { Component, OnInit } from '@angular/core';
import { FormArray, FormGroup, AbstractControl } from '@angular/forms';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { SortEvent } from 'primeng/components/common/api';
import { ControllerService } from '@app/services/controller.service';
import { TableUtility } from './tableutility';

// declare var google: any;

/**
 * Basic component that all components should extend from
 */
export class BaseComponent implements OnInit {
  // protected static googleLoaded: any;
  private _interval: any;

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService) {
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

}
