/**-----
 Angular imports
 ------------------*/
 import { async, ComponentFixture, TestBed, tick, fakeAsync, discardPeriodicTasks, flush } from '@angular/core/testing';
 import { HttpClientTestingModule } from '@angular/common/http/testing';
 import { Component } from '@angular/core';
 import { configureTestSuite } from 'ng-bullet';
 import { FormsModule, ReactiveFormsModule } from '@angular/forms';
 import { MatIconRegistry, MatTabsModule } from '@angular/material';
 import { NoopAnimationsModule } from '@angular/platform-browser/animations';
 import { RouterTestingModule } from '@angular/router/testing';
 import { AlerttableService } from '@app/services/alerttable.service';
 import { SharedModule } from '@app/components/shared/shared.module';

 /**-----
  Venice web-app imports
  ------------------*/
 import { ControllerService } from '@app/services/controller.service';
 import { ConfirmationService } from 'primeng/primeng';
 import { LogPublishersService } from '@app/services/logging/log-publishers.service';
 import { LogService } from '@app/services/logging/log.service';
 import { MaterialdesignModule } from '@lib/materialdesign.module';
 import { PrimengModule } from '@lib/primeng.module';
 import { EventsService } from '@app/services/events.service';
 import { EventsService as EventsServiceGen } from '@app/services/generated/events.service';
 import { SearchService } from '@app/services/generated/search.service';
 import { UIConfigsService } from '@app/services/uiconfigs.service';
 import { BehaviorSubject } from 'rxjs';
 import { Utility } from '@app/common/Utility';
 import { EventsEvent, EventsEventAttributes_severity, ApiListWatchOptions_sort_order, EventsEvent_type } from '@sdk/v1/models/generated/events';
 import { Eventsv1Service } from '@sdk/v1/services/generated/eventsv1.service';
 import { By } from '@angular/platform-browser';
 import { TestingUtility } from '@app/common/TestingUtility';
 import { MonitoringService } from '@app/services/generated/monitoring.service';
 import { MessageService } from '@app/services/message.service';
 import { AuthService } from '@app/services/auth.service';
 import { EventstableComponent } from './eventstable.component';

@Component({
  template: ''
})
class DummyComponent { }

function genEvent(modTime, creationTime, severity, type, message, source, count) {
  const _ = Utility.getLodash();
  return new EventsEvent({
    'meta': {
      'creation-time': creationTime,
      'mod-time': modTime,
    },
    'severity': severity,
    'type': type,
    'message': message,
    'object-ref': {
      kind: 'Node',
      name: 'test-node-1'
    },
    'source': source,
    'count': count
  });
}

describe('EventstableComponent', () => {
  const observer = new BehaviorSubject<any>([]);
  const poll1 = [];
  poll1.push(genEvent(
    '2018-08-20T19:09:04.777255798Z',
    '2018-08-20T19:09:02.777255798Z',
    'info', EventsEvent_type.AUDITING_FAILED, 'Service pen-evtsmgr running on node1',
    { component: 'pen-evtsmgr', 'node-name': 'node1' }, 1));

  let component: EventstableComponent;
  let fixture: ComponentFixture<EventstableComponent>;
  let eventsService: EventsService;
  let uiconfigService: UIConfigsService;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [
        DummyComponent,
      ],
      imports: [
        RouterTestingModule.withRoutes([]),
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
        SharedModule,
        MatTabsModule,
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        AlerttableService,
        MatIconRegistry,
        EventsService,
        EventsServiceGen,
        Eventsv1Service,
        AlerttableService,
        SearchService,
        UIConfigsService,
        AuthService,
        MonitoringService,
        MessageService
      ]
    });
      });

  beforeEach(() => {
    eventsService = TestBed.get(EventsService);
    uiconfigService = TestBed.get(UIConfigsService);
    spyOn(uiconfigService, 'isAuthorized').and.returnValue(true);
    spyOn(uiconfigService, 'roleGuardIsAuthorized').and.returnValue(true);
    spyOn(eventsService, 'pollEvents').and.returnValue(observer);
    fixture = TestBed.createComponent(EventstableComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('should have events in the table', <any>fakeAsync(() => {
    expect(eventsService.pollEvents).toHaveBeenCalledWith('alertsevents', { 'sort-order': ApiListWatchOptions_sort_order.none, 'field-selector': 'severity!=debug' });
    // Switch to the events tab
    // const tabs = fixture.debugElement.queryAll(By.css('.mat-tab-label'));
    // expect(tabs.length).toBe(2);
    // tabs[1].nativeElement.click();
    // tabs[1].children[0].nativeElement.click();
    fixture.detectChanges();
    tick(20000);
    fixture.detectChanges();
    fixture.whenRenderingDone().then(() => {
      // Table should be blank right now
      const eventsContainer = fixture.debugElement.query(By.css('.alertsevents-events-table'));
      const title = eventsContainer.query(By.css('.tableheader-title'));
      expect(title.nativeElement.textContent).toContain('Events (0)');

      // Checking table header
      const headers = eventsContainer.query(By.css('thead tr .sorticon-content'));
      headers.children.forEach((col, index) => {
        expect(col.nativeElement.textContent).toContain(component.cols[index].header);
      });

      // Checking that table is blank
      let tableBody = eventsContainer.query(By.css('.ui-table-scrollable-body-table tbody'));
      expect(tableBody.children.length).toBe(0);

      // Table has one event
      observer.next(poll1);
      fixture.detectChanges();

      // Checking that the table entry is there
      tableBody = eventsContainer.query(By.css('.ui-table-scrollable-body-table tbody'));
      fixture.whenRenderingDone().then(() => {
        // Allows rendering for the setTimeout(.., 0) we have
        setTimeout(() => {
          fixture.detectChanges();
          expect(tableBody.children.length).toBe(1);
          const caseMap = {
            'severity': (field, rowData, rowIndex) => {
              expect(field.children.length).toBe(2);
              if (rowData.severity === 'info') {
                expect(field.children[0].nativeElement.textContent).toContain('notifications');
              } else {
                expect(field.children[0].nativeElement.textContent).toContain('error');
              }
              expect(field.children[1].nativeElement.textContent)
                .toContain(EventsEventAttributes_severity[rowData.severity],
                  'severity column did not match');
            },
            'object-ref': (field, rowData, rowIndex) => {
              expect(field.nativeElement.textContent).toContain(
                rowData['object-ref'].kind + ' : ' + rowData['object-ref'].name, 'object-ref column did not match');
            },
            'source': (field, rowData, rowIndex) => {
              expect(field.nativeElement.textContent).toContain(
                rowData.source['node-name'] + ' : ' + rowData.source.component, 'source column did not match');
            }
          };
          TestingUtility.verifyTable(poll1, component.cols, tableBody, caseMap);
        }, 0);
      });
    });
    discardPeriodicTasks();
    flush();
    fixture.destroy();
  }));
});
