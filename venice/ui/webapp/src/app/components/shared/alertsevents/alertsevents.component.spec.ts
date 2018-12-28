/**-----
 Angular imports
 ------------------*/
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { Component } from '@angular/core';
import { async, ComponentFixture, TestBed, fakeAsync, tick, discardPeriodicTasks } from '@angular/core/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { AlerttableService } from '@app/services/alerttable.service';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { AlertseventsComponent } from './alertsevents.component';
import { LazyrenderComponent } from '@app/components/shared/lazyrender/lazyrender.component';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { EventsService } from '@app/services/events.service';
import { EventsService as EventsServiceGen } from '@app/services/generated/events.service';
import { SearchService } from '@app/services/generated/search.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { WhitespaceTrimDirective } from '@app/components/shared/directives/whitespacetrim.directive';
import { TableheaderComponent } from '@app/components/shared/tableheader/tableheader.component';
import { BehaviorSubject } from 'rxjs';
import { Utility } from '@app/common/Utility';
import { EventsEvent, EventsEventAttributes_severity_uihint } from '@sdk/v1/models/generated/events';
import { Eventsv1Service } from '@sdk/v1/services/generated/eventsv1.service';
import { By } from '@angular/platform-browser';
import { TestingUtility } from '@app/common/TestingUtility';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { MessageService } from 'primeng/primeng';
import { SorticonComponent } from '../sorticon/sorticon.component';


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
    'source': source,
    'count': count
  });
}

describe('AlertseventsComponent', () => {
  const observer = new BehaviorSubject<any>([]);
  const poll1 = [];
  poll1.push(genEvent(
    '2018-08-20T19:09:04.777255798Z',
    '2018-08-20T19:09:02.777255798Z',
    'INFO', 'ServiceRunning', 'Service pen-evtsmgr running on node1',
    { component: 'pen-evtsmgr', 'node-name': 'node1' }, 1));

  let component: AlertseventsComponent;
  let fixture: ComponentFixture<AlertseventsComponent>;
  let eventsService: EventsService;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [
        AlertseventsComponent,
        LazyrenderComponent,
        DummyComponent,
        PrettyDatePipe,
        TableheaderComponent,
        WhitespaceTrimDirective,
        SorticonComponent
      ],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
      ],
      providers: [
        ControllerService,
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
        MonitoringService,
        MessageService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    eventsService = TestBed.get(EventsService);
    spyOn(eventsService, 'pollEvents').and.returnValue(observer);
    fixture = TestBed.createComponent(AlertseventsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should have events in the table', async(() => {
    expect(eventsService.pollEvents).toHaveBeenCalledWith('alertsevents', {});
    // Switch to the events tab
    const tabs = fixture.debugElement.queryAll(By.css('.mat-tab-label'));
    expect(tabs.length).toBe(2);
    tabs[1].nativeElement.click();
    // tabs[1].children[0].nativeElement.click();
    fixture.detectChanges();
    fixture.whenRenderingDone().then(() => {
      // Table should be blank right now
      const eventsContainer = fixture.debugElement.query(By.css('.mat-tab-body-active .alertsevents-events-table'));
      const title = eventsContainer.query(By.css('.mat-tab-body-active .tableheader-title'));
      expect(title.nativeElement.textContent).toContain('Events (0)');

      // Checking table header
      const headers = eventsContainer.query(By.css('thead tr .sorticon-content'));
      headers.children.forEach((col, index) => {
        expect(col.nativeElement.textContent).toContain(component.eventCols[index].header);
      });

      // Checking that table is blank
      let tableBody = eventsContainer.query(By.css('.mat-tab-body-active .ui-table-scrollable-body-table tbody'));
      expect(tableBody.children.length).toBe(0);

      // Table has one event
      observer.next(poll1);
      fixture.detectChanges();

      // Checking that the table entry is there
      tableBody = eventsContainer.query(By.css('.mat-tab-body-active .ui-table-scrollable-body-table tbody'));
      fixture.whenRenderingDone().then(() => {
        // Allows rendering for the setTimeout(.., 0) we have
        setTimeout(() => {
          fixture.detectChanges();
          expect(tableBody.children.length).toBe(1);
          const caseMap = {
            'severity': (field, rowData, rowIndex) => {
              expect(field.children.length).toBe(2);
              if (rowData.severity === 'INFO') {
                expect(field.children[0].nativeElement.textContent).toContain('notifications');
              } else {
                expect(field.children[0].nativeElement.textContent).toContain('error');
              }
              expect(field.children[1].nativeElement.textContent)
                .toContain(EventsEventAttributes_severity_uihint[rowData.severity],
                  'severity column did not match');
            },
            'source': (field, rowData, rowIndex) => {
              expect(field.nativeElement.textContent).toContain(
                rowData.source['node-name'] + ' : ' + rowData.source.component, 'source column did not match');
            }
          };
          TestingUtility.verifyTable(poll1, component.eventCols, tableBody, caseMap);
        }, 0);
      });
    });

  }));
});
