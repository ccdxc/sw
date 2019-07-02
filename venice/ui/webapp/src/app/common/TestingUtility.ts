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

export class TestingUtility {
  fixture: ComponentFixture<any>;

  public static createWatchEventsSubject(obj: any[]) {
    return new BehaviorSubject(TestingUtility.createWatchEvents(obj));
  }

  public static createWatchEvents(obj: any[]) {
    const events = [];
    obj.forEach( (o) => {
      events.push({
        type: 'Created',
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
      } = {}, actionColContent = '') {

    const rows = tableElem.queryAll(By.css('tr'));
    expect(rows.length).toBe(data.length, 'Data did not match number of entries in the table');
    rows.forEach((row, rowIndex) => {
      const rowData = data[rowIndex];
      row.children.forEach((field, fieldIndex) => {
        if (fieldIndex === columns.length) {
          // Action col
          expect(field.nativeElement.textContent).toBe(actionColContent, 'action column did not match');
          return;
        }
        const colData = columns[fieldIndex];
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
