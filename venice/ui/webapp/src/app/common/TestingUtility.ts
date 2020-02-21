import { DebugElement } from '@angular/core';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { By } from '@angular/platform-browser';
import { Utility } from '@app/common/Utility';
import { } from 'jasmine';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { BehaviorSubject } from 'rxjs';
import { ControllerService } from '@app/services/controller.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { IMonitoringSyslogExport, MonitoringSyslogExportConfig_facility_override_uihint, MonitoringEventPolicySpec_format_uihint, IFieldsRequirement } from '@sdk/v1/models/generated/monitoring';

export class TestingUtility {
  fixture: ComponentFixture<any>;

  public static createDataCacheSubject(objList: any[], eventList: any[] = [], iconnIsErrorState: boolean = false) {
    const events = TestingUtility.createWatchEvents(eventList);
    const obj = {
      'data':  objList,
      'events': events.events,
      'connIsErrorState': iconnIsErrorState
    };
    return new BehaviorSubject(obj);
  }

  public static createWatchEventsSubject(obj: any[]) {
    return new BehaviorSubject(TestingUtility.createWatchEvents(obj));
  }



  public static createWatchEvents(obj: any[], type: string = 'Created') {
    const events = [];
    obj.forEach((o) => {
      events.push({
        type: type,
        object: o
      });
    });
    return { events: events };
  }

  public static createListResponse(obj: any[]) {
    return new BehaviorSubject({
      body: {
        items: obj
      }
    });
  }

  public static updateRoleGuards() {
    const service = TestBed.get(ControllerService);
    service.publish(Eventtypes.NEW_USER_PERMISSIONS, null);
  }

  public static setAllPermissions() {
    const serviceAny = TestBed.get(UIConfigsService) as any;
    serviceAny.uiPermissions = {};
    Object.keys(UIRolePermissions).forEach((p) => {
      serviceAny.uiPermissions[p] = true;
    });
    this.updateRoleGuards();
  }

  public static removeAllPermissions() {
    const serviceAny = TestBed.get(UIConfigsService) as any;
    serviceAny.uiPermissions = {};
    this.updateRoleGuards();
  }

  public static addPermissions(permissions: UIRolePermissions[]) {
    const serviceAny = TestBed.get(UIConfigsService) as any;
    permissions.forEach((p) => {
      serviceAny.uiPermissions[p] = true;
    });
    this.updateRoleGuards();
  }

  public static removePermissions(permissions: UIRolePermissions[]) {
    const serviceAny = TestBed.get(UIConfigsService) as any;
    permissions.forEach((p) => {
      delete serviceAny.uiPermissions[p];
    });
    this.updateRoleGuards();
  }

  /**
   * Returning the equality instead of using a jasmine expect so
   * that the caller can add context to the expect call.
   */
  public static isDateDisplayCorrect(time, timeDebugElem): boolean {
    const formattedModTime = new PrettyDatePipe('en-US').transform(time);
    return timeDebugElem.nativeElement.textContent.indexOf(formattedModTime) >= 0;
  }


  /**
   * Verifies the data in the table matches the passed in data.
   * If the value is meta or a basic printing of the value, it
   * will automatically be verified.
   * For other fields, pass in custom checking through the caseMap
   */
  public static verifyTable(data: any[], columns: any[],
    tableElem: DebugElement,
    caseMap:
      {
        [key: string]:
        (fieldElem: DebugElement, rowData: any, rowIndex: number) => void
      } = {},
    actionColContent = '',
    hasCheckbox: boolean = false) {

    const rows = tableElem.queryAll(By.css('tr'));
    expect(rows.length).toBe(data.length, 'Data did not match number of entries in the table');
    rows.forEach((row, rowIndex) => {
      const rowData = data[rowIndex];
      row.children.forEach((field, fieldIndex) => {
        if (hasCheckbox && fieldIndex === 0) {
          return;
        }
        // check whether table contains checkbox and reach to action icon column
        if ((hasCheckbox && fieldIndex === columns.length + 1) || (!hasCheckbox && fieldIndex === columns.length)) {
          // Action col
          expect(field.nativeElement.textContent).toBe(actionColContent, 'action column did not match');
          return;
        }
        const myFieldIndex = (hasCheckbox) ? fieldIndex - 1 : fieldIndex; // compute the right column index based on whether table contains checkbox.
        const colData = columns[myFieldIndex];
        if (caseMap[colData.field] != null) {
          // Delegate to supplied function
          const handlerFunction = caseMap[colData.field];
          handlerFunction(field, rowData, rowIndex);
        } else {
          switch (colData.field) {
            case 'meta.mod-time':
              expect(
                TestingUtility.isDateDisplayCorrect(rowData.meta['mod-time'], field)
              ).toBeTruthy('mod-time incorrect for row' + rowIndex);
              break;

            case 'meta.creation-time':
              expect(
                TestingUtility.isDateDisplayCorrect(rowData.meta['creation-time'], field)
              ).toBeTruthy('creation-time incorrect for row' + rowIndex);
              break;

            case 'meta.labels':
              if (rowData.meta.labels != null) {
                const labels = Object.keys(rowData.meta.labels);
                labels.forEach(label => {
                  expect(field.nativeElement.textContent)
                    .toContain(label + ': ' + rowData.meta.labels[label], 'labels column did not contain ' + label + ' for row ' + rowIndex);
                });
              }
              break;
            default:
              const fieldData = Utility.getObjectValueByPropertyPath(rowData, colData.field.split('.'));
              expect(field.nativeElement.textContent).toContain(fieldData, colData.header + ' did not match for row ' + rowIndex);
          }
        }
      });
    });
  }


  constructor(fixture: ComponentFixture<any>) {
    this.fixture = fixture;
  }

  setText(elem: DebugElement, text: string) {
    const elemNative = elem.nativeElement;
    elemNative.value = text;
    elemNative.dispatchEvent(new Event('input'));
    this.fixture.detectChanges();
    return this.fixture.whenStable();
  }

  sendEnterKeyup(elem: DebugElement) {
    elem.triggerEventHandler('keyup', { keyCode: 13 });
    this.fixture.detectChanges();
    return this.fixture.whenStable();
  }

  sendClick(elem: DebugElement) {
    const elemNative = elem.nativeElement;
    elemNative.click();
    this.fixture.detectChanges();
    return this.fixture.whenStable();
  }


  isDateDisplayCorrect(time, timeDebugElem): boolean {
    return TestingUtility.isDateDisplayCorrect(time, timeDebugElem);
  }

  getElemByCss(css: string) {
    return this.fixture.debugElement.query(By.css(css));
  }

  getElemsByCss(css: string) {
    return this.fixture.debugElement.queryAll(By.css(css));
  }

  setDropdown(dropdownCss, label, dropdownOptionCSS: string = null) {
    const arrowCss = dropdownCss + ' .ui-dropdown-trigger > span';
    const arrowElem = this.getElemByCss(arrowCss);
    expect(arrowElem).toBeTruthy('Arrow elem was not found: ' + arrowCss);
    this.sendClick(arrowElem);
    const opCSS = (dropdownOptionCSS) ? dropdownOptionCSS : '.ng-trigger.ng-trigger-overlayAnimation.ui-dropdown-panel.ui-widget .ui-dropdown-items-wrapper .ui-dropdown-item > span';
    const options = this.getElemsByCss(opCSS);
    const selectedElem = options.find((elem) => {
      return elem.nativeElement.textContent.includes(label);
    });
    expect(selectedElem).toBeTruthy('Could not find the given option: ' + label);
    this.sendClick(selectedElem);
  }

  setInput(elemCss, text) {
    this.setText(this.getElemByCss(elemCss), text);
  }

  setRadioButton(radioGroupCss, label) {
    const options = this.getElemsByCss(radioGroupCss + ' .mat-radio-label-content');
    const selectedElem = options.find((elem) => {
      return elem.nativeElement.textContent.includes(label);
    });
    expect(selectedElem).toBeTruthy('Could not find the given option: ' + label);
    this.sendClick(selectedElem);
  }

  setSyslogData(data: IMonitoringSyslogExport) {
    this.setDropdown('.syslog-override', MonitoringSyslogExportConfig_facility_override_uihint[data.config['facility-override']]);
    this.setInput('.syslog-prefix', data.config.prefix);
    this.setRadioButton('.syslog-format', MonitoringEventPolicySpec_format_uihint[data.format]);
    if (data.targets.length > 1) {
      fail('currently only supports adding one syslog target');
    } else if (data.targets.length === 1) {
      this.setInput('.syslog-destination', data.targets[0].destination);
      this.setInput('.syslog-transport', data.targets[0].transport);
    }
  }

  setRepeater(data: IFieldsRequirement[]) {
    if (data.length === 0) {
      return;
    } else if (data.length > 1) {
      fail('currently only one requirement supported');
    }
    this.setDropdown('.repeater-key p-dropdown', data[0].key);
    let operator: any = data[0].operator;
    if (operator === 'in') {
      operator = 'equals';
    } else if (operator === 'notIn') {
      operator = 'not equals';
    }
    this.setDropdown('.repeater-operator p-dropdown', operator);
    if (this.getElemByCss('.repeater-input-value')) {
      this.setInput('.repeater-input-value', data[0].values.join(', '));
    } else if (this.getElemByCss('.repeater-value p-dropdown')) {
      this.setDropdown('.repeater-value p-dropdown', data[0].operator);
    } else if (this.getElemByCss('.repeater-value p-multiSelect')) {
      fail('setting multiselect is unimplemented');
    } else {
      fail('did not find repeater value element');
    }
  }

  /**
   * Verifies the data in the table matches the passed in data.
   * If the value is meta or a basic printing of the value, it
   * will automatically be verified.
   * For other fields, pass in custom checking through the caseMap
   */
  verifyTable(data: any[], columns: any[],
    tableElem: DebugElement,
    caseMap:
      {
        [key: string]:
        (fieldElem: DebugElement, rowData: any, index: number) => void
      }) {
    TestingUtility.verifyTable(data, columns, tableElem, caseMap);
  }


}
