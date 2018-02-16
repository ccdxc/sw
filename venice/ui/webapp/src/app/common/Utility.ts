import { ControllerService } from '../services/controller.service';
import { LogService } from '../services/logging/log.service';
import { Eventtypes } from '../enum/eventtypes.enum';
import { environment } from '../../environments/environment';
import * as _ from 'lodash';
declare var $: any;


export class Utility {

  static instance: Utility;

  // alernate comments on/off the following two lines to use on-line or off-line REST API
  // public static isOffLine = false;
  public static isOffLine = true;

  myControllerService: ControllerService;
  myLogService: LogService;

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

  static getGUID() {
    function s4() {
      return Math.floor((1 + Math.random()) * 0x10000)
        .toString(16)
        .substring(1);
    }
    return s4() + s4() + '-' + s4() + '-' + s4() + '-' + s4() + '-' + s4() + s4() + s4();
  }

  static s4() {
    return Math.floor((1 + Math.random()) * 0x10000)
      .toString(16)
      .substring(1);
  }


  static getPayloadDatetime() {
    const d = new Date();
    return d.toISOString();
  }

  static getPastDate(minuusDay): Date {
    const d = new Date();
    d.setDate(d.getDate() - minuusDay);
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
    // server may give date string as'2014-07-06T00:00:00.000+0000', IE and Safari can deal with that.  Thus, we have a util function to parse it Date object
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

  static makeFirstLetterUppercase(inputString: string): string {
    return inputString.charAt(0).toUpperCase() + inputString.slice(1);
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

  static isRESTFailed(res: any): Boolean {
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

  static vitalsDataSortHelper(a, b): number {
    const aDate = new Date(a['date']);
    const bDate = new Date(b['date']);
    return aDate.getTime() - bDate.getTime();
  }

  static getTimeDifferenceDuration(diff): string {
    const dayDiff = Math.floor(diff / 1000 / 3600 / 24);
    const hourDiff = Math.floor(diff / 1000 / 60 / 24);
    const minutesDiff = Math.floor(diff / 1000 / 60);
    const diffUnit = '';
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
   * Sample reference is in ICDView.component.ts
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

  static isEmpty(val) {
    return (val === undefined || val === null || val.length <= 0) ? true : false;
  }

  /**
 * Compare two JSON objects and remove non-object properties
 *  A is { "key": "1", "obj":{ "A2":"A2"}}
 *  vs
 *  B is { "key": "1", "obj":{ "B2":"B2"}}
 *  ==
 *   A will be { "obj":{ "A2":"A2"}}  // property "key" is removed
 */
  static _trimUnchangedAttributes(checkObj: any, tgtObj: any) {
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

  static getLodash(): any {
    return _;
  }

  static getJQuery(): any {
    return $;
  }

  static comparyDatePart(myDate: Date, compareToDate: Date): number {
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
    const nVer = navigator.appVersion;
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
            const _key = i.replace(/{/, '').replace(/}/, '');
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



  private constructor() { }

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




}
