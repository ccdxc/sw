import { HttpErrorResponse } from '@angular/common/http';
import { AbstractControl, FormArray, FormGroup, ValidatorFn, ValidationErrors } from '@angular/forms';
import { SearchExpression } from '@app/components/search/index.ts';
import { AUTH_BODY, AUTH_KEY } from '@app/core/auth/auth.reducer';
import { CategoryMapping } from '@sdk/v1/models/generated/category-mapping.model';
import { FieldsRequirement_operator, IFieldsSelector, IApiObjectMeta } from '@sdk/v1/models/generated/monitoring';
import { StagingBuffer, StagingCommitAction } from '@sdk/v1/models/generated/staging';
import * as $ from 'jquery';
import * as _ from 'lodash';
import * as moment from 'moment';
import * as pluralize from 'pluralize';
import { SelectItem } from 'primeng/primeng';
import { Table } from 'primeng/table';
import { RepeaterComponent } from 'web-app-framework';
import { Eventtypes } from '../enum/eventtypes.enum';
import { ControllerService } from '../services/controller.service';
import { LogService } from '../services/logging/log.service';
import { UIConfigsService } from '../services/uiconfigs.service';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { ClusterDistributedServiceCard, ClusterDSCCondition, ClusterDSCCondition_status, ClusterDSCCondition_type, ClusterNode } from '@sdk/v1/models/generated/cluster';
import { ILabelsSelector, RolloutRollout } from '@sdk/v1/models/generated/rollout';
import { NaplesCondition, NaplesConditionValues, NodeConditionValues } from '@app/components/cluster-group/naples/index.ts';
import { IAuthUser } from '@sdk/v1/models/generated/auth';
import { ClusterNodeCondition_type, ClusterNodeCondition_status, ClusterDistributedServiceCardStatus_admission_phase } from '@sdk/v1/models/generated/cluster';
import { HeroCardOptions } from '@app/components/shared/herocard/herocard.component';



export class Utility {

  static instance: Utility;

  // Define Valid Mac address format
  public static MACADDRESS_REGEX =
    /^([0-9a-fA-F]{4}[.]){2}([0-9a-fA-F]{4})$/;

  // Define Valid time out format
  public static TIMEOUT_REGEX = /^(\d+(h|m|s|ms|ns|us|µs))+$/;

  // Determines wheter to use on-line or off-line REST API
  public static isOffLine = true;
  public static XSRF_NAME = 'Grpc-Metadata-Csrf-Token';

  // Define a  // VS-302
  public static LOGIN_IDLE_SETTIME_HANDLE: any = null;
  // Key for the observable obtained from localStorage.getUserdataObservable
  public static USER_DATA_OBSERVABLE = 'UserDataObservable';
  public static MAX_OBJECT_NAME_LENGTH = 64;
  public static WEBSOCKET_403 = 'Please check your authorization settings and contact system administrator.';  // see VS=157
  public static UPDATE_FAILED_SUMMARY = 'Update Failed';
  public static CREATE_FAILED_SUMMARY = 'Create Failed';
  public static DELETE_FAILED_SUMMARY = 'Delete Failed';
  public static UPDATE_SUCCESS_SUMMARY = 'Update Successful';
  public static CREATE_SUCCESS_SUMMARY = 'Create Successful';
  public static DELETE_SUCCESS_SUMMARY = 'Delete Successful';
  public static VENICE_CONNECT_FAILURE_SUMMARY = 'Failed to connect to Venice';
  public static ROLLOUT_IMGAGE_NAMESPACE = 'images';
  // In RBAC, when kind is ['AuditEvent', 'FwlogsQuery', 'Event'] , there is no group
  public static KINDS_WITHOUT_GROUP = ['AuditEvent', 'FwlogsQuery', 'Event'];

  public static HERO_CARD_THIRDVALUE_LENGTH: number = 15;

  myControllerService: ControllerService;
  myLogService: LogService;
  myUIConfigsService: UIConfigsService;

  private _maintenanceMode: boolean = false;
  private _currentRollout: RolloutRollout = null;

  private constructor() { }

  /**
  * Takes in a list of objects, and reads their labels to build a map
  * Mapping is from label key to a map of label values which maps
  * to object names that have that label.
  */
  public static getLabels(objList: any[]): {
    [labelKey: string]:
    { [labelValue: string]: string[] }
  } {
    const labels = {};
    objList.forEach((obj) => {
      if (obj == null || obj.meta == null || obj.meta.labels == null) {
        return;
      }
      Object.keys(obj.meta.labels).forEach((key) => {
        const value = obj.meta.labels[key];
        let currVals = labels[key];
        if (currVals == null) {
          currVals = {};
          labels[key] = currVals;
        }
        let objNames = currVals[value];
        if (objNames == null) {
          objNames = [];
          currVals[value] = objNames;
        }
        objNames.push(obj.meta.name);
      });
    });
    return labels;
  }

  /**
   * Convert
   * {"order-constraints":[{"requirements":[{"key":"number","operator":"notEquals","values":["-1"]},{"key":"number ","operator":"notEquals","values":["3"]}]}]} to
   * number notEquals -1
   * number notEquals 3
   * @param labelsSelectors;
   */
  public static convertLabelSelectorsToStringList(labelsSelectors: ILabelsSelector[]): string[] {
    let list = [];
    if (!labelsSelectors) {
      return [];
    }
    labelsSelectors.forEach((labelsSelector) => {
      const myList = Utility.convertOneLabelSelectorToStringList(labelsSelector);
      list = list.concat(myList);
    });
    return list;
  }

  public static convertOneLabelSelectorToStringList(labelsSelector: ILabelsSelector): string[] {
    const labelsRequirements = labelsSelector.requirements;
    const list = [];
    if (!labelsSelector) {
      return [];
    }
    labelsRequirements.forEach(
      (labelsRequirement) => {
        if (!Utility.isEmpty(labelsRequirement.key) && !Utility.isEmpty(labelsRequirement.operator) && !Utility.isEmpty(labelsRequirement.values)) {
          const str = labelsRequirement.key + ' ' + labelsRequirement.operator + ' ' + labelsRequirement.values + '\n';
          list.push(str);
        }
      }
    );
    return list;
  }

  /**
   * If the user hasn't specified a value, we don't add
   * it into the values we return to the consumer.
   * If a field is free form text, we split by comma to turn
   * it into an array of values
   */
  public static formatRepeaterData(data: any[], valueFormName: string = 'valueFormControl') {
    if (data == null) {
      return null;
    }
    let retData = data.filter((item) => {
      return item[valueFormName] != null && item[valueFormName].length !== 0;
    });
    // make sure the value field is an array
    retData = retData.map((item) => {
      if (item[valueFormName] instanceof Array) {
        let arr = [];
        item[valueFormName].forEach(ele => {
          if (typeof ele === 'string' || ele instanceof String) {
            const arrVal = ele.split(',');
            arr = arr.concat(arrVal.map(val => {
              return val.trim();
            }));
          } else {
            arr.push(ele);
          }
        });
        item[valueFormName] = arr;
      } else {
        const arrVal = item[valueFormName].split(',');
        item[valueFormName] = arrVal.map(val => {
          return val.trim();
        });
      }
      return item;
    });
    return retData;
  }

  public static stringifyRepeaterData(data: IFieldsSelector[], keyFormName: string = 'keyFormControl', operatorFormName: string = 'operatorFormControl', valueFormName: string = 'valueFormControl'): string[] {
    if (data == null) {
      return [];
    }
    const ret = [];
    data.forEach((item) => {
      const key = item[keyFormName];
      let operator = item[operatorFormName];
      switch (operator) {
        case 'equals':
        case 'in':
          operator = '=';
          break;
        case 'notEquals':
        case 'notIn':
          operator = '!=';
          break;
        case 'gt':
          operator = '>';
          break;
        case 'gte':
          operator = '>=';
          break;
        case 'lt':
          operator = '<';
          break;
        case 'lte':
          operator = '<=';
          break;
        default:
          break;
      }
      const values = item[valueFormName].join(', ');
      const res = `${key} ${operator} ${values}`;
      ret.push(res);
    });
    return ret;
  }

  public static getAuditEventCacheSize(): number {
    return 10; // For future code refactoring, we are planning to use user-preferences and env.variable
  }

  public static getHeroCardDisplayValue(value: string, maxLen: number = this.HERO_CARD_THIRDVALUE_LENGTH): string {
    if (value && value.length > maxLen) {
      return value.substring(0, 15) + '...';
    }
    return value;
  }

  public static customizeHeroCardThirdStat(thirdStat: string, heroCard: HeroCardOptions, thirdStatName: string) {
    if (thirdStat.length > Utility.HERO_CARD_THIRDVALUE_LENGTH) {
      if (heroCard.thirdStat.tooltip.indexOf(thirdStatName) < 0) {
        heroCard.thirdStat.tooltip += ' (' + thirdStatName + ')';
      }
    }
  }

  /**
   * get REST API server URL
   * In production enviornment, UI is hosted along with API-Gateway. Thus, UI url is that same as API-GW url.
   * In dev enviorment, we proxy all requests to the server specified in proxy.conf.js
   */
  static getRESTAPIServerAndPort(): string {
    return window.location.protocol + '//' + window.location.hostname + ':' + window.location.port;
  }

  static getDocURL(): string {
    return Utility.getRESTAPIServerAndPort() + '/docs/';
  }

  static getBaseUIUrl(): string {
    return window.location.protocol + '//' + window.location.hostname + ':' + window.location.port + '/#/';
  }

  static isIE(): boolean {
    // IF IE > 10
    if ((navigator.userAgent.indexOf('MSIE') !== -1) || (!!document['documentMode'] === true)) {
      return true;
    }
    return false;
  }

  static getLocationHashObject(): any {
    let loc = window.location.hash;
    const idx = loc.indexOf('?');
    if (idx >= 0) {
      loc = loc.substr(1 + idx);
    }
    // e.g "loc=austria&mr=1&min=10&max=89"
    const hash2Obj = loc
      .split('&')
      .map(el => el.split('='))
      .reduce((pre, cur) => { pre[cur[0]] = cur[1]; return pre; }, {});
    return hash2Obj;
  }



  static getRandomInt(min: number, max: number) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
  }

  public static getRandomIntList(len: number, start: number, end: number): any {
    const list = [];
    for (let i = 0; i < len; i++) {
      const value = this.getRandomInt(start, end);
      list.push(value);
    }
    return list;
  }

  /**
   * Returns the complimentary values from 100
   * Ex: [20, 30, 45] --> [80, 70, 55]
   * @param list
   */
  public static getComplimentaryList(list): any {
    const newList = [];

    for (let i = 0; i < list.length; i++) {
      const value = 100 - list[i];
      newList.push(value);
    }
    return newList;
  }

  static getGUID() {
    const s4 = this.s4;
    return s4() + s4() + '-' + s4() + '-' + s4() + '-' + s4() + '-' + s4() + s4() + s4();
  }

  static s4() {
    return Math.floor((1 + Math.random()) * 0x10000)
      .toString(16)
      .substring(1);
  }

  static getMacAddress(): string {
    return 'XX:XX:XX:XX:XX:XX'.replace(/X/g, function () {
      return '0123456789ABCDEF'.charAt(Math.floor(Math.random() * 16));
    });
  }

  static getIPv4(): string {
    const ipv4 = (Math.floor(Math.random() * 255) + 1) + '.' + (Math.floor(Math.random() * 255) + 0) + '.' + (Math.floor(Math.random() * 255) + 0) + '.' + (Math.floor(Math.random() * 255) + 0);
    return ipv4;
  }

  static getPayloadDatetime() {
    const d = new Date();
    return d.toISOString();
  }

  static getPastDate(minusDay): Date {
    const d = new Date();
    d.setDate(d.getDate() - minusDay);
    d.setHours(0);
    d.setMinutes(0);
    d.setSeconds(0);
    return d;
  }

  static clearHourMinuteSecond(d: Date) {
    d.setHours(0);
    d.setMinutes(0);
    d.setSeconds(0);
    return d;
  }

  static convertToDate(dateInput: any): Date {
    // server may give date string as'2014-07-06T00:00:00.000+0000',
    // IE and Safari can't deal with that.
    // Thus, we have a util function to parse it Date object
    let d;
    if (dateInput instanceof Date) {
      return dateInput;
    }
    try {
      d = new Date(dateInput);
      return d;
    } catch (e) {
      console.error(dateInput + ' can not be parsed to date');
    }

    const str1 = dateInput.split('T')[0];
    try {
      d = new Date(str1);
    } catch (e) {
      console.error(dateInput + ' can not be parsed to date');
    }
    return d;
  }

  static makeFirstLetterUppercase(inputString: string, firstCharOnly: boolean = false): string {
    if (!inputString) {
      return inputString;
    }
    return (!firstCharOnly) ? inputString.charAt(0).toUpperCase() + inputString.slice(1) : inputString.charAt(0).toUpperCase();
  }

  static isValidDate(date: any): boolean {
    return (date instanceof Date && !isNaN(date.valueOf()));
  }

  static computeAge(dateOfBirth: string): number {
    const today = new Date();
    const nowyear = today.getFullYear();

    const birth = new Date(dateOfBirth);
    const birthyear = birth.getFullYear();
    const age = nowyear - birthyear;
    return age;
  }

  static isRESTSuccess(res: any): Boolean {
    return (res['result'] !== 'failure');
  }

  static getRESTMessage(res: any): string {
    return res['message']['error'];
  }

  static getRESTMessageCode(res: any): string {
    return res['message']['errorCode'];
  }

  static getRandomDate(rangeOfDays: number, startHour: number, hourRange: number, isMinus: Boolean = true): Date {
    const today = new Date();
    const direction = (isMinus) ? -1 : 1;
    return new Date(today.getFullYear(), today.getMonth(), today.getDate() + Math.random() * direction * rangeOfDays, Math.random() * hourRange + startHour, Math.random() * 60);
  }

  /**
   *
   * @param field Field to sort by
   * @param order 1 for asc, -1 for dec
   */
  static dateSortHelper(aObject: any, bObject: any, fields: any, order: number): number {
    const value1 = _.get(aObject, fields, 0);
    const value2 = _.get(bObject, fields, 0);

    let result = 0;

    if (value1 === null && value2 !== null) {
      result = -1;
    } else if (value1 !== null && value2 === null) {
      result = 1;
    } else if (value1 === null && value2 === null) {
      result = 0;
    } else {
      const aDate = new Date(value1).getTime();
      const bDate = new Date(value2).getTime();

      if (aDate < bDate) {
        result = -1;
      } else if (aDate > bDate) {
        result = 1;
      }
    }
    return (result * order);
  }

  /**
   * Sort objects by date
   * @param list
   * @param fields
   * @param order
   *
   * For example:
   *  VS-630 sort the alerts in desc order
   *  this.alerts = Utility.sortDate(this.alerts, ['meta', 'mod-time'], -1);
   *
   */
  static sortDate(list: any, fields: string[], order: number): any[] {
    if (order !== 1 && order !== -1) {
      console.error('Invalid sort order given');
      return list;
    }
    const sortedList = list.sort((a, b) => {
      return this.dateSortHelper(a, b, fields, order);
    });
    return sortedList;
  }

  static getTimeDifferenceDuration(diff): string {
    const dayDiff = Math.floor(diff / 1000 / 3600 / 24);
    const hourDiff = Math.floor(diff / 1000 / 60 / 24);
    const minutesDiff = Math.floor(diff / 1000 / 60);
    let unit = '';
    if (dayDiff > 0) {
      unit = (dayDiff === 1) ? 'day' : 'days';
      unit = dayDiff + ' ' + unit;
    } else {
      if (hourDiff > 0) {
        unit = (hourDiff === 1) ? 'hour' : 'hours';
        unit = hourDiff + ' ' + unit;
      } else {
        unit = (minutesDiff === 1) ? 'minute' : 'minutes';
        unit = minutesDiff + ' ' + unit;
      }
    }
    return unit;
  }

  // encode(decode) html text into html entity
  static decodeHtmlEntity(str: string) {
    return str.replace(/&#(\d+);/g, function (match, dec) {
      return String.fromCharCode(dec);
    });
  }

  static encodeHtmlEntity(str) {
    const buf = [];
    for (let i = str.length - 1; i >= 0; i--) {
      buf.unshift(['&#', str[i].charCodeAt(), ';'].join(''));
    }
    return buf.join('');
  }

  static escape(s): any {
    return s.replace(/[&"<>]/g, function (c) {
      return {
        '&': '&amp;',
        '"': '&quot;',
        '<': '&lt;',
        '>': '&gt;'
      }[c];
    });
  }

  /**
   * This is API recursively traverse the JSON hiearchy. I will invoke callback functions.
   *
   * objectCallback and nonObjectCallback are functions.
   * linkComponent is a compontent.ts
   */
  static traverseJSONObject(data: any, objectCallback: any, nonObjectCallback: any, linkComponent: any) {
    for (const key in data) {
      if (typeof (data[key]) === 'object' && data[key] !== null) {
        if (Array.isArray(data[key])) {
          objectCallback(data, data[key], key, -1, linkComponent);
          for (let i = 0; i < data[key].length; i++) {
            if (objectCallback) {
              objectCallback(data[key], data[key][i], key, i, linkComponent);
            }
            this.traverseJSONObject(data[key][i], objectCallback, nonObjectCallback, linkComponent);
          }

        } else {
          if (objectCallback) {
            objectCallback(data, data[key], key, -1, linkComponent);
          }
          this.traverseJSONObject(data[key], objectCallback, nonObjectCallback, linkComponent);
        }
      } else {
        if (nonObjectCallback) {
          nonObjectCallback(data, data[key], key, linkComponent);
        }
      }
    }
  }

  /**
   * is the given item a leaf node
   */
  static isLeafNode(anItem: any): boolean {

    if (typeof (anItem) === 'function') { return false; }
    if (typeof (anItem) === 'object') { return false; }
    return true;
  }

  static isPlainObject(anItem: any): boolean {
    return this.getLodash().isPlainObject(anItem);
  }

  static isEmpty(val, checkTrim: boolean = false) {
    if (val === undefined || val === null || val.length <= 0) {
      return true;
    }
    if (!checkTrim) {
      return false;
    }
    return val.trim().length === 0;
  }

  public static isEmptyObject(obj: any): boolean {
    return this.getLodash().isEmpty(obj);
  }

  public static isPortValid(port: string): boolean {
    if (!port || !port.trim()) {
      return false;
    }
    const number = Number(port.trim());
    if (isNaN(number)) {
      return false;
    }
    if (number < 1 || number > 65536) {
      return false;
    }
    return true;
  }

  // this one checks whether the string is a port, a port range or
  // a list of ports or port ranges seperated by comma
  // the method returns a string as error message
  // returns null if there are no errors
  public static isPortsValid(val: string, delimiter: string = ','): string {
    const arr: string[] = val.trim().split(delimiter);
    for (let i = 0; i < arr.length; i++) {
      const port: string = arr[i].trim();
      const portArr: string[] = port.split('-');
      if (portArr.length > 2) {
        return port + ' is an invalid port address or port range.';
      }
      if (portArr.length === 1) {
        if (!this.isPortValid(portArr[0])) {
          return portArr[0] + ' is an invalid port address.';
        }
      } else { // portArr.length === 2
        if (!this.isPortValid(portArr[0])) {
          return portArr[0] + ' is an invalid port address.';
        }
        if (!this.isPortValid(portArr[1])) {
          return portArr[1] + ' is an invalid port address.';
        }
        if (parseInt(portArr[0], 10) >= parseInt(portArr[1], 10)) {
          return port + ' is an invalid port range.';
        }
      }
    }
    return null;
  }

  public static isTimeoutValid(timeout: string): boolean {
    return this.TIMEOUT_REGEX.test(timeout);
  }

  public static getMousePosition(event) {
    let posx = 0;
    let posy = 0;
    let e = event;
    if (!event) {
      e = window.event;
    }
    if (e.pageX || e.pageY) {
      posx = e.pageX;
      posy = e.pageY;
    } else if (e.clientX || e.clientY) {
      posx = e.clientX + document.body.scrollLeft + document.documentElement.scrollLeft;
      posy = e.clientY + document.body.scrollTop + document.documentElement.scrollTop;
    }
    return { 'x': posx, 'y': posy };
  }

  /**
 * Compare two JSON objects and remove non-object properties
 *  A is { "key": "1", "obj":{ "A2":"A2"}}
 *  vs
 *  B is { "key": "1", "obj":{ "B2":"B2"}}
 *  ==
 *   A will be { "obj":{ "A2":"A2"}}  // property "key" is removed
 */
  public static _trimUnchangedAttributes(checkObj: any, tgtObj: any) {
    for (const attr in checkObj) {
      if (Utility.isLeafNode(checkObj[attr])) {
        const chkValue = checkObj[attr];
        const tgtValue = (tgtObj) ? tgtObj[attr] : null;
        if (chkValue === tgtValue) {
          delete checkObj[attr];
        }
      }
    }
  }

  /**
     * traverse tow JSONs to compare node by node and remove non-changed nodes.
     * use lodash API
     *
     */
  static compareTwoJSONObjects(data: any, refData: any) {
    for (const key in data) {
      if (typeof (data[key]) === 'object' && data[key] !== null && refData) {
        if (Array.isArray(data[key])) {
          // if the array is equal, delete it. otherwise, preserve the array
          const isEqual = _.isEqual(data[key], refData[key]);
          if (isEqual) {
            delete data[key];
          }
        } else {
          // compare object. If equal, remove it. Otherwise, travse the sub-tree
          const isEqual = _.isEqual(data[key], refData[key]);
          if (isEqual) {
            delete data[key];
          } else {
            this.compareTwoJSONObjects(data[key], refData[key]);
          }
        }

      } else {
        // remove unchange non-obj attributes
        const chkValue = data[key];
        const tgtValue = (refData) ? refData[key] : null;
        if (chkValue === null && tgtValue === null) {
          delete data[key];
        } else {
          this._trimUnchangedAttributes(data, refData);
        }
      }
    }
  }

  static getLodash(): _.LoDashStatic {
    return _;
  }

  static getMomentJS(): any {
    return moment;
  }

  static getJQuery(): any {
    return $;
  }

  static getPluralize(): any {
    return pluralize;
  }

  static compareDatePart(myDate: Date, compareToDate: Date): number {
    const d1 = myDate.setHours(0, 0, 0, 0);
    const d2 = compareToDate.setHours(0, 0, 0, 0);
    if (d1 === d2) {
      return 0;
    } else if (d1 > d2) {
      return 1;
    } else {
      return -1;
    }
  }

  static getOperatingSystem(): string {
    // http://www.javascripter.net/faq/operatin.htm

    // This script sets OSName variable as follows:
    // "Windows"    for all versions of Windows
    // "MacOS"      for all versions of Macintosh OS
    // "Linux"      for all versions of Linux
    // "UNIX"       for all other UNIX flavors
    // "Unknown OS" indicates failure to detect the OS

    let OSName = 'UnknownOS';
    if (navigator.appVersion.indexOf('Win') !== -1) { OSName = 'Windows'; }
    if (navigator.appVersion.indexOf('Mac') !== -1) { OSName = 'MacOS'; }
    if (navigator.appVersion.indexOf('X11') !== -1) { OSName = 'UNIX'; }
    if (navigator.appVersion.indexOf('Linux') !== -1) { OSName = 'Linux'; }
    return OSName;
  }

  static getBrowserInfomation(): Object {
    // const nVer = navigator.appVersion;
    const nAgt = navigator.userAgent;
    let browserName = navigator.appName;
    let fullVersion = '' + parseFloat(navigator.appVersion);
    let majorVersion = parseInt(navigator.appVersion, 10);
    let nameOffset, verOffset, ix;

    // In Opera 15+, the true version is after "OPR/"
    if ((verOffset = nAgt.indexOf('OPR/')) !== -1) {
      browserName = 'Opera';
      fullVersion = nAgt.substring(verOffset + 4);
    } else if ((verOffset = nAgt.indexOf('Opera')) !== -1) {
      // In older Opera, the true version is after "Opera" or after "Version"

      browserName = 'Opera';
      fullVersion = nAgt.substring(verOffset + 6);
      if ((verOffset = nAgt.indexOf('Version')) !== -1) {
        fullVersion = nAgt.substring(verOffset + 8);
      }
    } else if ((verOffset = nAgt.indexOf('MSIE')) !== -1) {
      // In MSIE, the true version is after "MSIE" in userAgent

      browserName = 'Microsoft Internet Explorer';
      fullVersion = nAgt.substring(verOffset + 5);
    } else if ((verOffset = nAgt.indexOf('Chrome')) !== -1) {
      // In Chrome, the true version is after "Chrome"

      browserName = 'Chrome';
      fullVersion = nAgt.substring(verOffset + 7);
    } else if ((verOffset = nAgt.indexOf('Safari')) !== -1) {
      // In Safari, the true version is after "Safari" or after "Version"

      browserName = 'Safari';
      fullVersion = nAgt.substring(verOffset + 7);
      if ((verOffset = nAgt.indexOf('Version')) !== -1) {
        fullVersion = nAgt.substring(verOffset + 8);
      }
    } else if ((verOffset = nAgt.indexOf('Firefox')) !== -1) {
      // In Firefox, the true version is after "Firefox"

      browserName = 'Firefox';
      fullVersion = nAgt.substring(verOffset + 8);
    } else if ((nameOffset = nAgt.lastIndexOf(' ') + 1) < (verOffset = nAgt.lastIndexOf('/'))) {
      // In most other browsers, "name/version" is at the end of userAgent
      browserName = nAgt.substring(nameOffset, verOffset);
      fullVersion = nAgt.substring(verOffset + 1);
      if (browserName.toLowerCase() === browserName.toUpperCase()) {
        browserName = navigator.appName;
      }
    }
    // trim the fullVersion string at semicolon/space if present
    if ((ix = fullVersion.indexOf(';')) !== -1) {
      fullVersion = fullVersion.substring(0, ix);
    }
    if ((ix = fullVersion.indexOf(' ')) !== -1) {
      fullVersion = fullVersion.substring(0, ix);
    }

    majorVersion = parseInt('' + fullVersion, 10);
    if (isNaN(majorVersion)) {
      fullVersion = '' + parseFloat(navigator.appVersion);
      majorVersion = parseInt(navigator.appVersion, 10);
    }

    const obj = {};
    obj['fullVersion'] = fullVersion;
    obj['browserName'] = browserName;
    obj['majorVersion'] = majorVersion;
    obj['appName'] = navigator.appName;
    obj['userAgent'] = navigator.userAgent;

    return obj;
  }

  static convertToUTC(myDate: any): number {
    let longTimeNumber = myDate;
    if (myDate instanceof Date) {
      longTimeNumber = myDate.getTime();
    }
    const d = new Date();
    const n = d.getTimezoneOffset();  // PST - UTC is 420, It is 420 minutes
    const sign = (n > 0) ? -1 : 1;
    return longTimeNumber + (sign) * n * 60 * 1000;
  }

  static copyContentToClipboard(val: string) {
    const selBox = document.createElement('textarea');
    selBox.style.position = 'fixed';
    selBox.style.left = '0';
    selBox.style.top = '0';
    selBox.style.opacity = '0';
    selBox.value = val;

    document.body.appendChild(selBox);
    selBox.focus();
    selBox.select();

    document.execCommand('copy');
    document.body.removeChild(selBox);
  }

  /**
   * Find distinct item from an array
   * input "array" is object list
   * field is a property of the object.
   *
   * For example,  array is [{age:3}, {age:4}, {age:3}]
   *  getUniqueValueFromArray(array, "age") --> [3,4] (if idOny==true)  else --> [{age:3}, {age:4}]
   *
   */
  static getUniqueValueFromArray(array: any, field: string, idOnly: boolean = true): any {
    const unique = {};
    const distinct = [];
    for (const i in array) {
      if (array.hasOwnProperty(i)) {
        if (typeof (unique[array[i][field]]) === 'undefined') {
          if (idOnly) {
            distinct.push(array[i][field]);
          } else {
            distinct.push(array[i]);
          }
        }
        unique[array[i][field]] = 0;
      }
    }
    return distinct;
  }

  /**
   * build an array of selectItems
   * input "array" is simple string array
   */
  static buildSelectItemsFromArray(array: any): any {
    const selectItems = [];
    for (const i in array) {
      if (array.hasOwnProperty(i)) {
        const obj = {};
        obj['label'] = array[i];
        obj['value'] = array[i];
        selectItems.push(obj);
      }
    }
    return selectItems;
  }

  public static getInstance(): Utility {
    if (!this.instance) {
      this.instance = new Utility();
    }
    return this.instance;
  }

  /**
   * var string = Utility.stringInject("This is a {0} string for {1}", ["test", "stringInject"]);
   *  output is
   *  This is a test string for stringInject
   *
   *  var str = Utility.stringInject("My username is {username} on {platform}", { username: "tjcafferkey", platform: "GitHub" });
   *
   *  My username is tjcafferkey on Github
   */
  public static stringInject(str, data): string {
    if (typeof str === 'string' && (data instanceof Array)) {

      return str.replace(/({\d})/g, function (i) {
        return data[i.replace(/{/, '').replace(/}/, '')];
      });
    } else if (typeof str === 'string' && (data instanceof Object)) {

      for (const key in data) {
        if (data.hasOwnProperty(key)) {
          return str.replace(/({([^}]+)})/g, function (i) {
            i.replace(/{/, '').replace(/}/, '');
            if (!data[key]) {
              return i;
            }

            return data[key];
          });
        }
      }
    } else {
      return str;
    }
  }

  /**
   * This API traverse object TREE to fetch value
   * Unless specified, will automatically apply return the ui hints
   * version of the value if possible
   */
  public static getObjectValueByPropertyPath(inputObject: any, fields: string | Array<string>, useUIHints: boolean = true): any {
    if (fields == null) {
      return null;
    }
    let value = inputObject;
    let uiHintMap;
    if (useUIHints && inputObject.getPropInfo != null) {
      // We have an object from venice-sdk, so we can find its property info
      const propInfo = this.getNestedPropInfo(inputObject, fields);
      uiHintMap = (propInfo) ? propInfo.enum : null;
    }
    value = _.get(inputObject, fields);
    if (uiHintMap != null) {
      if (Array.isArray(value)) {
        value = value.map((v) => uiHintMap[v]);
      } else {
        value = uiHintMap[value];
      }
    }
    return value;
  }

  /**
   * Gets the propinfo for the given key.
   */
  public static getNestedPropInfo(instance: any, keys: string | Array<string>, reportMissingPropInf: boolean = true) {
    if (keys == null) {
      return null;
    }
    if (typeof keys === 'string') {
      keys = keys.split('.');
    }
    const parent = _.get(instance, keys.slice(0, -1));
    let propInfo = null;
    if (parent != null) {
      propInfo = parent.getPropInfo(keys[keys.length - 1]);
    } else {
      propInfo = instance.getPropInfo(keys[keys.length - 1]);
    }
    if (propInfo == null && reportMissingPropInf) {
      console.error('propInfo was null, supplied property path is likely invalid. Input keys and instance: ' + keys + ' ' + ((instance.kind) ? instance.kind : instance));
    }
    return propInfo;
  }

  public static convertEnumToSelectItem(enumVal: object, skipVals = []): SelectItem[] {
    const ret: SelectItem[] = [];
    for (const key in enumVal) {
      if (enumVal.hasOwnProperty(key)) {
        if (!skipVals.includes(key)) {
          const value = enumVal[key];
          ret.push({ label: value, value: key });
        }
      }
    }
    return ret;
  }

  /**
   * Follow
   *  pensando/sw/venice/ui/venice-sdk/v1/models/generated/category-mapping.model.ts
   */

  public static getCategories(): any[] {
    let cats = Object.keys(CategoryMapping);
    cats = cats.sort();
    return cats;
  }

  public static getKinds(): any[] {
    const cats = Utility.getCategories();
    let kinds = [];
    cats.filter((cat) => {
      const catKeys = Object.keys(CategoryMapping[cat]);
      kinds = kinds.concat(catKeys);
    });
    kinds = kinds.sort();
    return kinds;
  }

  /**
   * see api/generated/apiclient/svcmanifest.json
   * @param isValueToLowerCase
   */
  public static convertCategoriesToSelectItem(isValueToLowerCase: boolean = true): SelectItem[] {
    const cats = this.getCategories();
    return this.stringArrayToSelectItem(cats, isValueToLowerCase);
  }

  public static convertKindsToSelectItem(isValueToLowerCase: boolean = false): SelectItem[] {
    const kinds = this.getKinds();
    return this.stringArrayToSelectItem(kinds, isValueToLowerCase);
  }

  public static stringArrayToSelectItem(stringArray: string[], isValueToLowerCase: boolean = true): SelectItem[] {
    const ret: SelectItem[] = [];
    for (let i = 0; i < stringArray.length; i++) {
      const value = stringArray[i];
      ret.push({ label: value, value: isValueToLowerCase ? value.toLocaleLowerCase() : value });
    }
    return ret;
  }

  public static getKindsByCategory(selectedCategory: string): any[] {
    const obj = CategoryMapping[selectedCategory];
    if (!obj) {
      return [];
    }
    return Object.keys(obj);
  }

  /**
   * Find category from kind.
   * e.g given "Node" as a kind, return "Cluster" as category.
   * see pensando/sw/venice/ui/venice-sdk/v1/models/generated/category-mapping.model.ts
   * @param kind
   */
  public static findCategoryByKind(kind: string): string {
    const category = null;
    const cats = Object.keys(CategoryMapping);
    for (let i = 0; i < cats.length; i++) {
      const cat = cats[i];
      const kinds = this.getKindsByCategory(cat);
      for (let j = 0; j < kinds.length; j++) {
        if (kind === kinds[j]) {
          return cat;
        }
      }
    }
    return category;
  }

  /**
   * This API check if object self-link has correspoonding UI URL
   * @param kind
   * @param name
   */
  public static isObjectSelfLinkHasUILink(kind: any, name: string): boolean {
    const route = this.genSelfLinkUIRoute(kind, name);
    return (route != null);
  }

  /**
   * This API convert object self-link to UI page link;
   * For example, search result dispaly auth-policy link as '/configs/v1/auth/authn-policy'. It will convert to UI page URL 'settings/authpolicy'
   */
  public static genSelfLinkUIRoute(kind: any, name: string, isToUseDefault: boolean = false): string {
    let cat = this.findCategoryByKind(kind);
    cat = (cat) ? cat.toLowerCase() : '';
    switch (kind) {
      case 'Cluster':
        return cat + '/cluster';
      case 'Node':
        return cat + '/cluster' + (name ? '/' + name : '');
      case 'DistributedServiceCard':
        return cat + '/dscs';
      case 'Host':
        return cat + '/hosts';
      case 'Workload':
      case 'Endpoint':
        return 'workload';  // VS-363, when search is:Workload or is:Endpoint, we should /workload page
      case 'Alert':
        return cat + '/alertsevents/alert/' + name;
      case 'Event':
        return cat + '/alertsevents/event/' + name;
      case 'AlertPolicy':
        return cat + '/alertsevents/alertpolicies';
      case 'AlertDestination':
        return cat + '/alertsevents/alertdestinations';
      case 'AuthenticationPolicy':
        return 'admin/authpolicy';
      case 'App':
        return 'security/securityapps';
      case 'NetworkSecurityPolicy':
        return 'security/sgpolicies';
      case 'AuditEvent':
        return 'monitoring/auditevents';
      case 'FwlogPolicy':
        return 'monitoring/fwlogs/fwlogpolicies';
      case 'FlowExportPolicy':
        return 'monitoring/flowexport';
      case 'User':
      case 'Role':
      case 'RoleBinding':
        return 'admin/users';
      case 'Rollout':
        return 'admin/upgrade/rollouts/' + name;
      case 'EventPolicy':
        return 'monitoring/alertsevents/eventpolicy';
      case 'SnapshotRestore':
          return 'admin/snapshots'; // VS-1059
      default:
        return (!isToUseDefault) ? null : cat + '/' + pluralize.plural(kind.toLowerCase()) + '/' + name;
    }
  }

  /**
   * Returns a map from key values to series
   */
  public static splitDataIntoSeriesByKey(dataValues, keyIndex) {
    return _.groupBy(dataValues, (item) => {
      return item[keyIndex];
    });
  }

  public static transformToPlotly(data, xFieldIndex, yFieldIndex) {
    const x = [];
    const y = [];
    data.forEach((item) => {
      x.push(item[xFieldIndex]);
      y.push(item[yFieldIndex]);
    });
    return { x: x, y: y };
  }


  public static scaleBytes(bytes: number, decimals = 2, unit: string = '') {
    const k = 1024;
    const dm = decimals < 0 ? 0 : decimals;
    const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];
    let i = sizes.findIndex(s => s === unit);
    if (i == null) {
      i = Math.floor(Math.log(bytes) / Math.log(k));
    }

    return parseFloat((bytes / Math.pow(k, i)).toFixed(dm));
  }

  public static getBytesType(bytes: number, decimals = 2) {
    const k = 1024;
    const dm = decimals < 0 ? 0 : decimals;
    const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));

    return sizes[i];
  }

  /**
   * @param bytes amount in bytes to format
   * @param decimals number of decimal places
   * @param maxLength max length of string ('.' is not counted)
   */
  public static formatBytes(bytes: number, decimals = 2, maxLength = 10) {
    if (maxLength < 3) {
      // if we have 800 kb, we need at least 3 digits
      // to show it. We could format that as 0.8 MB, but
      // the logic for that isn't here yet, and there isn't
      // a use case for it yet.
      console.error('max length must be at least three');
      maxLength = 10;
    }
    if (bytes === 0) { return '0 Bytes'; }
    const k = 1024,
      dm = decimals <= 0 ? 0 : decimals || 2,
      sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'],
      i = Math.floor(Math.log(bytes) / Math.log(k));
    const val = parseFloat((bytes / Math.pow(k, i)).toFixed(dm));

    if (isNaN(val)) {
      return '0 bytes';
    }

    let valStr = val.toString();
    // not counting . for length
    while (valStr.replace('.', '').length > maxLength) {
      valStr = valStr.slice(0, valStr.length - 1);
    }
    if (valStr.endsWith('.')) {
      valStr = valStr.slice(0, valStr.length - 1);
    }
    return valStr + ' ' + sizes[i];
  }

  public static average(arr) {
    return arr.reduce((a, b) => a + b, 0) / arr.length;
  }

  public static buildCommitBuffer(): StagingBuffer {
    const data = {
      'kind': 'Buffer',
      'meta': {
        'name': Utility.s4() + Utility.s4(),
        'tenant': Utility.getInstance().getTenant(),
        'namespace': Utility.getInstance().getNamespace()
      },
      'spec': {}
    };
    return new StagingBuffer(data, false);
  }

  public static buildCommitBufferCommit(buffername: string): StagingCommitAction {
    const data = {
      'kind': 'CommitAction',
      'meta': {
        'name': buffername,
        'tenant': Utility.getInstance().getTenant(),
        'namespace': Utility.getInstance().getNamespace()
      },
      'spec': {}
    };
    return new StagingCommitAction(data, false);
  }

  /**
   * Rounds the time down to the nearest min
   * @param min Nearest min to round down to
   * @param time time to round, if blank is the current time
   */
  public static roundDownTime(min, time: Date = new Date): Date {
    const coeff = 1000 * 60 * min;
    return new Date(Math.floor(time.getTime() / coeff) * coeff);
  }

  /**
   * This API traverse the form group to check if every control is validation error free.
   * @param form: FormGroup | FormArray
   */
  public static getAllFormgroupErrors(form: FormGroup | FormArray): { [key: string]: any; } | null {
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

  /**
   * This API generate tooltip message for form control based on validation result.
   * For example
   * 1. In authpolicy radius.server.secret field, secret validation rule is "required && minlen >= 1"
   * Utililty.getControlTooltip (server,'secret', 'please enter secret, min-len >=5' ); // see html to, 'server' is the control
   *
   * 2. In authpolicy ladp.base-password  field
   * Utililty.getControlTooltip (server, null, 'please enter password, min-len >=8' );
   */
  public static getControlTooltip(control: AbstractControl, field: string, defaultTooltip: string): string {
    if (control) {
      const targetControl = (field) ? control.get(field) : control;
      if (targetControl && targetControl.errors) {
        return JSON.stringify(targetControl.errors);
      }
    }
    return (defaultTooltip) ? defaultTooltip : (field) ? field : '';
  }

  /**
   * This API export data. Exported data will be downloaded to user machine.
   * @param content (like csv string.  You can )
   * @type: 'text/csv;charset=utf-8;'
   * @param exportFilename (like table.csv)
   */
  public static exportContent(content: any, type: string, exportFilename: string, ) {
    const blob = new Blob([content], {
      type: type
    });

    if (window.navigator.msSaveOrOpenBlob) {
      navigator.msSaveOrOpenBlob(blob, exportFilename);
    } else {
      const link = document.createElement('a');
      link.style.display = 'none';
      document.body.appendChild(link);
      if (link.download !== undefined) {
        link.setAttribute('href', URL.createObjectURL(blob));
        link.setAttribute('download', exportFilename);
        link.click();
      } else {
        content = 'data:text/csv;charset=utf-8,' + content;
        window.open(encodeURI(content));
      }
      document.body.removeChild(link);
    }
  }



  /**
   * This API extract table data to JSON
   * For example:
   * const json = Utility.extractTableContentToJSON(this.auditeventsTable);
   * console.log('json\n', json);
   * @param table - primeng table
   * @param options - json object {selectionOnly: boolean}
   */
  public static extractTableContentToJSON(table: Table, options?: any, ): any[] {
    let data = table.filteredValue || table.value;
    const output = [];
    const headers: string[] = [];
    if (options && options.selectionOnly) {
      data = table.selection || [];
    }
    // headers
    for (let i = 0; i < table.columns.length; i++) {
      const column = table.columns[i];
      if (column.exportable !== false && column.field) {
        headers.push((column.header || column.field));
      }
    }
    // body
    data.forEach((record, i) => {
      const rec = {};
      for (let j = 0; j < table.columns.length; j++) {
        const column = table.columns[j];
        if (column.exportable !== false && column.field) {
          let cellData = table.objectUtils.resolveFieldData(record, column.field);
          if (cellData != null) {
            if (table.exportFunction) {
              cellData = table.exportFunction({
                data: cellData,
                field: column.field
              });
            } else {
              cellData = String(cellData).replace(/"/g, '""');
            }
          } else {
            cellData = '';
          }
          if (!Utility.isEmpty(headers[j])) {
            rec[headers[j]] = cellData;
          }
        }
      }
      output.push(rec);
    });
    return output;
  }

  /**
   * This API extract table data to CVS
   *  For example:
   *  const csv = Utility.extractTableContentToCSV(this.auditeventsTable);
   *  console.log('csv\n', csv);
   *
   *  @param table is a primeNG Table object
   *  @param options (It looks like {selectionOnly: boolean})
   */
  public static extractTableContentToCSV(table: Table, options?: any, ): string {
    let data = table.filteredValue || table.value;
    let csv = '\ufeff';

    if (options && options.selectionOnly) {
      data = table.selection || [];
    }
    // headers
    for (let i = 0; i < table.columns.length; i++) {
      const column = table.columns[i];
      if (column.exportable !== false && column.field) {
        csv += '"' + (column.header || column.field) + '"';
        if (i < (table.columns.length - 1)) {
          csv += table.csvSeparator;
        }
      }
    }
    // body
    data.forEach((record, i) => {
      csv += '\n';
      for (let j = 0; j < table.columns.length; j++) {
        const column = table.columns[j];
        if (column.exportable !== false && column.field) {
          let cellData = table.objectUtils.resolveFieldData(record, column.field);
          if (cellData != null) {
            if (table.exportFunction) {
              cellData = table.exportFunction({
                data: cellData,
                field: column.field
              });
            } else {
              cellData = String(cellData).replace(/"/g, '""');
            }
          } else {
            cellData = '';
          }
          csv += '"' + cellData + '"';
          if (j < (table.columns.length - 1)) {
            csv += table.csvSeparator;
          }
        }
      }
    });
    return csv;
  }

  /**
   * This API convert csv string to an array of json object
   * For example
   * const csv = Utility.extractTableContentToCSV(this.auditeventsTable);
   * console.log('csv\n', csv);
   * const csv2json = Utility.csvToObjectArray(csv);
   * console.log('csv2json\n', csv2json);
   *
   * @param csvString
   */
  public static csvToObjectArray(csvString: string): any[] {
    const csvRowArray = csvString.split(/\n/);
    const headerCellArray = this.trimQuotes(csvRowArray.shift().split(','));
    const objectArray = [];

    while (csvRowArray.length) {
      const rowCellArray = this.trimQuotes(csvRowArray.shift().split(','));
      const rowObject = _.zipObject(headerCellArray, rowCellArray);
      objectArray.push(rowObject);
    }
    return objectArray;
  }

  public static trimQuotes(stringArray: string[]) {
    for (let i = 0; i < stringArray.length; i++) {
      stringArray[i] = _.trim(stringArray[i], '"');
    }
    return stringArray;
  }

  public static getFieldOperatorSymbol(opString: string): string {
    switch (opString) {
      case FieldsRequirement_operator.equals:
        return '=';
        break;
      case FieldsRequirement_operator.notequals:
        return '!=';
        break;
      case FieldsRequirement_operator.gt:
        return '>';
        break;
      case FieldsRequirement_operator.gte:
        return '>=';
        break;
      case FieldsRequirement_operator.lte:
        return '<=';
        break;
      case FieldsRequirement_operator.lt:
        return '<';
        break;
      case FieldsRequirement_operator.in:
        return '=';
        break;
      case FieldsRequirement_operator.notin:
        return '!=';
        break;
      default:
        return opString;
    }
  }

  /**
   *
   * [{"keyFormControl":"text","operatorFormControl":"equals","valueFormControl":"1.2","keytextFormName":"version"}]
   */
  public static convertRepeaterValuesToSearchExpression(repeater: RepeaterComponent, addMetatag: boolean = false): SearchExpression[] {
    const data = repeater.getValues();
    if (data == null) {
      return null;
    }
    let retData = data.filter((item) => {
      return !Utility.isEmpty(item[repeater.keyFormName]) && !Utility.isEmpty(item[repeater.valueFormName]) && item[repeater.valueFormName].length !== 0;
    });
    // make sure the value field is an array
    retData = retData.map((item) => {
      const searchExpression: SearchExpression = {
        key: ((addMetatag) ? 'meta.labels.' : '') + item[repeater.keytextFormName],
        operator: item[repeater.operatorFormName],
        values: Array.isArray(item[repeater.valueFormName]) ? item[repeater.valueFormName] : item[repeater.valueFormName].trim().split(',')
      };
      return searchExpression;
    });
    return retData;
  }

  public static displayColumn(data, col, hasUiHintMap: boolean = true): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields, hasUiHintMap);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  public static displaySimpleObject(obj: Object, delimiter: string = ':'): string {
    if (!obj) {
      return '';
    }
    const list = [];
    Object.keys(obj).forEach((key) => {
      let ret = '';
      ret += key + delimiter + obj[key];
      list.push(ret);
    });
    return list.join(',');
  }

  // Walks the object and if a nested object has only null or defaults, it is
  // removed fromt he request
  // Request should be an object from Venice_sdk
  // This is useful for removing nested objects that have validation that
  // only apply if an object is given (Ex. pagination spec for telemetry queries)
  // TODO: Create a BaseType for this function to take in
  public static TrimDefaultsAndEmptyFields(request: any) {
    request = _.cloneDeep(request);
    // if helperFunc returns true, we delete the entry
    const helperFunc = (obj): boolean => {
      let retValue = true;
      for (const key in obj) {
        if (obj.hasOwnProperty(key)) {
          if (_.isObjectLike(obj[key])) {
            if (helperFunc(obj[key])) {
              delete obj[key];
            } else {
              retValue = false;
            }
          } else if (obj[key] != null) {
            retValue = false;
          }
        }
      }
      return retValue;
    };
    helperFunc(request);
    return request;
  }

  public static generateDeleteConfirmMsg(objectType: string, name: string): string {
    return 'Are you sure you want to delete ' + objectType + ': ' + name;
  }

  /**
   * When using forkjoin, we have to loop through the results to see if all results are successful
   */
  public static isForkjoinResultAllOK(results: any[]): boolean {
    let isAllOK: boolean = true;
    for (let i = 0; i < results.length; i++) {
      if (results[i]['statusCode'] === 200) {
        // debug here. Do nothing
      } else {
        isAllOK = false;
        break;
      }
    }
    return isAllOK;
  }

  public static joinErrors(results: any[]) {
    let errorCode = 400;

    const err = results.filter((r) => {
      if (errorCode === 400) {
        if (r.statusCode === 401) {
          errorCode = 401;
        } else if (r.statusCode === 403) {
          errorCode = 403;
        } else if (r.statusCode >= 500) {
          errorCode = 500;
        }
      }
      return r.statusCode !== 200;
    }).map((r) => {
      if (r.body != null) {
        return r.body.message;
      }
      return '';
    }).join('\n');
    const retError = {
      statusCode: errorCode,
      body: {
        message: err
      }
    };
    return retError;
  }

  public static dateValidator(compareToDate: Date, sign: number, objectname: string, msg: string): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      const noGood = {
        objectname: {
          required: true,
          message: msg
        }
      };
      if (!control.value) {
        return noGood;
      }
      const inputDate = new Date(control.value);
      const diff = this.compareDate(inputDate, compareToDate);
      if (diff !== sign) {
        return noGood;
      }
      return null;
    };
  }

  /**
  * Compares two Date objects and returns e number value that represents
  * the result:
  * 0 if the two dates are equal.
  * 1 if the first date is greater than second.
  * -1 if the first date is less than second.
  * @param date1 First date object to compare.
  * @param date2 Second date object to compare.
  */
  public static compareDate(date1: Date, date2: Date): number {
    // With Date object we can compare dates them using the >, <, <= or >=.
    // The ==, !=, ===, and !== operators require to use date.getTime(),
    // so we need to create a new instance of Date with 'new Date()'
    const d1 = new Date(date1); const d2 = new Date(date2);

    // Check if the dates are equal
    const same = d1.getTime() === d2.getTime();
    if (same) { return 0; }

    // Check if the first is greater than second
    if (d1 > d2) { return 1; }

    // Check if the first is less than second
    if (d1 < d2) { return -1; }
  }

  public static isModelNameUniqueValidator(existingTechSupportRequest: any[], objectname: string): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      if (this.isNameAlreadyExist(control.value, existingTechSupportRequest)) {
        return {
          objectname: {
            required: true,
            message: 'Name is required and must be unique.'
          }
        };
      }
      return null;
    };
  }

  public static isControlValueMatchOtherControlValueValidator(otherControl: AbstractControl, objectname: string, msg: string = 'Value not match'): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      if (control.value !== otherControl.value) {
        return {
          objectname: {
            required: true,
            message: msg
          }
        };
      }
      return null;
    };
  }

  public static isValueInRangeValdator(minValue: number, maxValue: number, objectname: string, msg: string = 'Value is out of range', delimitor: string = ','): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      if (!control || !control.value || control.value.length === 0) {
        return null;
      }
      const values = control.value;
      const valueList = values.split(delimitor);
      for (let i = 0; i < valueList.length; i++) {
        const value = valueList[i];
        if (value < minValue || value > maxValue) {
          return {
            objectname: {
              required: true,
              message: msg
            }
          };
        }
      }
      return null;
    };
  }


  public static isNameAlreadyExist(name: string, modelobjects: any[]): boolean {
    if (!modelobjects) {
      return false;
    }
    for (let i = 0; i < modelobjects.length; i++) {
      const tsrObj = modelobjects[i];
      if (tsrObj.meta.name === name) {
        return true;
      }
    }
    return false;
  }

  public static actionMethodNameToObject(methodName) {
    let ret;
    Object.keys(CategoryMapping).some((cat) => {
      return Object.keys(CategoryMapping[cat]).some((kind) => {
        const actions = CategoryMapping[cat][kind].actions.map(a => a.toLowerCase());
        if (actions.includes(methodName.toLowerCase())) {
          ret = {
            category: cat,
            kind: kind,
          };
          return true;
        }
        return false;
      });
    });
    return ret;
  }

  public static getNodeCondition(nodes: Readonly<ClusterNode>) {
    if (!nodes || !nodes.status || nodes.status.conditions == null || nodes.status.conditions.length === 0) {
      return NodeConditionValues.UNKNOWN;
    } else {
      // 2019-07-02 : conditions is technically an array, but backend only supplies one element in it, so this is fine
      for (const cond of nodes.status.conditions) {
        if ((cond) && (cond.type === ClusterNodeCondition_type.healthy) && (cond.status === ClusterNodeCondition_status.true)) {
          return NodeConditionValues.HEALTHY;
        } else if ((cond) && (cond.type === ClusterNodeCondition_type.healthy) && (cond.status === ClusterNodeCondition_status.false)) {
          return NodeConditionValues.UNHEALTHY;
        } else {
          return NodeConditionValues.UNKNOWN;
        }
      }
    }
  }

  public static getNaplesConditionObject(naples: Readonly<ClusterDistributedServiceCard>): NaplesCondition {
    if (!naples || naples.status['admission-phase'] !== ClusterDistributedServiceCardStatus_admission_phase.admitted) {
      return { isHealthy: false, condition: _.upperFirst(NaplesConditionValues.NOTADMITTED), rebootNeeded: this.isNaplesNeedReboot(naples) };
    } else if (naples.status.conditions == null || naples.status.conditions.length === 0) {
      return { isHealthy: false, condition: _.upperFirst(NaplesConditionValues.EMPTY), rebootNeeded: this.isNaplesNeedReboot(naples) };
    } else {
      for (const cond of naples.status.conditions) {
        if ((cond) && (cond.type === ClusterDSCCondition_type.healthy) && (cond.status === ClusterDSCCondition_status.false)) {
          return { isHealthy: false, condition: _.upperFirst(NaplesConditionValues.UNHEALTHY), rebootNeeded: this.isNaplesNeedReboot(naples) };
        }
        if ((cond) && (cond.type === ClusterDSCCondition_type.healthy) && (cond.status === ClusterDSCCondition_status.true)) {
          return { isHealthy: true, condition: _.upperFirst(NaplesConditionValues.HEALTHY), rebootNeeded: this.isNaplesNeedReboot(naples) };
        }
        if ((cond) && (cond.type === ClusterDSCCondition_type.healthy) && (cond.status === ClusterDSCCondition_status.unknown)) {
          return { isHealthy: false, condition: _.upperFirst(NaplesConditionValues.UNKNOWN), rebootNeeded: this.isNaplesNeedReboot(naples) };
        }
        if ((cond) && (cond.type === ClusterDSCCondition_type.nic_health_unknown)) {
          return { isHealthy: false, condition: _.upperFirst(NaplesConditionValues.UNKNOWN), rebootNeeded: this.isNaplesNeedReboot(naples) };
        }
      }
      return { isHealthy: false, condition: _.upperFirst(NaplesConditionValues.UNKNOWN), rebootNeeded: this.isNaplesNeedReboot(naples) };
    }
  }

  public static isNaplesNeedReboot(naples: Readonly<ClusterDistributedServiceCard>): boolean {
    if (naples) {
      for (const cond of naples.status.conditions) {
        if (cond && cond.type === ClusterDSCCondition_type.reboot_needed) {
          return true;
        }
      }
    }
    return false;
  }

  public static isNaplesNICHealthy(naples: Readonly<ClusterDistributedServiceCard>): boolean {
    return this.getNaplesConditionObject(naples).isHealthy;
  }

  public static isNaplesNICHealthUnknown(naples: Readonly<ClusterDistributedServiceCard>): boolean {
    const cond: NaplesCondition = this.getNaplesConditionObject(naples);
    return !cond.isHealthy && cond.condition === NaplesConditionValues.UNKNOWN;
  }

  public static isNaplesNICUnhealthy(naples: Readonly<ClusterDistributedServiceCard>): boolean {
    const cond: NaplesCondition = this.getNaplesConditionObject(naples);
    return !cond.isHealthy && cond.condition === NaplesConditionValues.UNHEALTHY;
  }

  public static getNaplesCondition(naples: Readonly<ClusterDistributedServiceCard>): string {
    return this.getNaplesConditionObject(naples).condition;
  }



  public static displayReasons(naples: Readonly<ClusterDistributedServiceCard>): string {
    const reasonarray: string[] = [];
    if (naples && naples.status && naples.status.conditions) {
      for (let i = 0; i < naples.status['conditions'].length; i++) {
        if (!this.isNaplesNICHealthy(naples)) {
          if (naples.status.conditions[i].reason != null) {
            const cond = naples.status.conditions[i];
            const reason = this.formatDateWithinString(cond);
            reasonarray.push(reason);
          }
        }
      }
    }
    if (reasonarray.length > 0) {
      return reasonarray.join('\n');
    } else {
      return '';
    }
  }


  public static isNICConditionNotAdmitted(naples: Readonly<ClusterDistributedServiceCard>): boolean { // If NIC not admitted, condition in table is left blank
    return this.getNaplesCondition(naples) === NaplesConditionValues.NOTADMITTED;
  }

  public static formatDateWithinString(cond: ClusterDSCCondition): string {
    const words: any[] = cond.reason.split(' ');
    for (let j = 0; j < words.length; j++) {    // parsing reason string to check for and format date/time events
      const w = words[j];
      const regex = /\d{4}-\d{2}-\d{2}.*/;
      if (w.match(regex) != null) {
        const date = new PrettyDatePipe('en-US').transform(w);
        words.splice(words.indexOf(w), 1, date);
      }
    }
    let reason = '';
    for (let k = 0; k < words.length; k++) {   // reconstructing new string
      reason = reason.concat(words[k], ' ');
    }
    return reason;
  }

  public static hexToRgb(hex: string) {
    if (hex.startsWith('#')) {
      hex = hex.slice(1);
    }
    const bigint = parseInt(hex, 16);
    /* tslint:disable:no-bitwise */
    const r = (bigint >> 16) & 255;
    const g = (bigint >> 8) & 255;
    const b = bigint & 255;
    /* tslint:enable:no-bitwise */
    return { r: r, g: g, b: b };
  }

  public static getRgbaFromHex(color: string, opacity) {
    const ret = Utility.hexToRgb(color);
    return `rgba(${ret.r}, ${ret.g}, ${ret.b}, ${opacity})`;
  }

  setControllerService(controllerService: ControllerService) {
    this.myControllerService = controllerService;
  }
  getControllerService(): ControllerService {
    return this.myControllerService;
  }

  setLogService(logService: LogService) {
    this.myLogService = logService;
  }

  getLogService(): LogService {
    return this.myLogService;
  }

  setUIConfigsService(uIConfigsService: UIConfigsService) {
    this.myUIConfigsService = uIConfigsService;
  }

  getUIConfigsService(): UIConfigsService {
    return this.myUIConfigsService;
  }

  publishAJAXEnd(payload: any) {
    if (this.myControllerService) {
      this.myControllerService.publish(Eventtypes.AJAX_END, { 'ajax': 'end', 'name': 'pv-AJAX' });
    }
  }

  publishAJAXStart(payload: any) {
    if (this.myControllerService) {
      this.myControllerService.publish(Eventtypes.AJAX_START, { 'ajax': 'start', 'name': 'pv-AJAX' });
    }
  }

  getTenant(): string {
    return 'default';
  }

  getNamespace(): string {
    return 'default';
  }

  getXSRFtoken(): string {
    const token = sessionStorage.getItem(AUTH_KEY);
    return token ? token : '';
  }

  getLoginUser(): any | null {
    const body = JSON.parse(sessionStorage.getItem(AUTH_BODY));
    if (body != null && body.meta != null) {
      return body;
    }
    return null;
  }

  isAdmin(): boolean {
    const user: IAuthUser = this.getLoginUser();
    if (user != null && user.status != null && user.status.roles != null && user.status.roles.includes('AdminRole')) {
      return true;
    }
    return false;
  }

  getLoginName(): string | null {
    const body = this.getLoginUser();
    if (body != null && body.meta != null) {
      return body.meta.name;
    }

    return null;
  }

  /**
   * This API will inform user to re-login to Venice. It is being used in this.getControllerService()
   * @param errorReponse
   */
  interceptHttpError(errorReponse: any | HttpErrorResponse) {
    if (errorReponse instanceof HttpErrorResponse) {
      // TODO: for now, it is for GS0.3
      const url = errorReponse.url;
      const isLoginURL: boolean = (url.indexOf('login') >= 0);
      if (errorReponse.status === 401 && !isLoginURL) {  // 401 is authentication error
        const r = confirm('Authentication credentials are no longer valid. Please login again.');
        if (r === true && this.getControllerService()) {
          this.getControllerService().publish(Eventtypes.LOGOUT, {});
        } else {
          return;
        }
      }
    }
  }

  getMaintenanceMode() {
    return this._maintenanceMode;
  }

  setMaintenanceMode(flag: boolean) {
    this._maintenanceMode = flag;
  }

  getCurrentRollout() {
    return this._currentRollout;
  }

  setCurrentRollout(rollout: RolloutRollout) {
    this._currentRollout = rollout;
  }

}
