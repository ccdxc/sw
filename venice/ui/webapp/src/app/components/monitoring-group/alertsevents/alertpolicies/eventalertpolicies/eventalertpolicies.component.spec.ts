/**-----
 Angular imports
 ------------------*/
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
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
import { EventalertpolicyComponent } from './eventalertpolicies.component';
import { NeweventalertpolicyComponent } from '@app/components/monitoring-group/alertsevents/alertpolicies/eventalertpolicies/neweventalertpolicy/neweventalertpolicy.component';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { WidgetsModule } from 'web-app-framework';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { TestingUtility } from '@app/common/TestingUtility';
import { By } from '@angular/platform-browser';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { AuthService } from '@app/services/auth.service';
import { MonitoringAlertPolicy } from '@sdk/v1/models/generated/monitoring';
import { BehaviorSubject } from 'rxjs';
import { TestTablevieweditRBAC } from '@app/components/shared/tableviewedit/tableviewedit.component.spec';

describe('EventalertpolicyComponent', () => {
  let component: EventalertpolicyComponent;
  let fixture: ComponentFixture<EventalertpolicyComponent>;
    let toolbarSpy: jasmine.Spy;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [EventalertpolicyComponent, NeweventalertpolicyComponent],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        WidgetsModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MonitoringService,
        MessageService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(EventalertpolicyComponent);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    fixture.detectChanges();
    expect(component).toBeTruthy();
  });

  describe('RBAC', () => {
    let testHelper = new TestTablevieweditRBAC('monitoringalertpolicy');

    beforeEach(() => {
      component.isActiveTab = true;
      component.dataObjects = [new MonitoringAlertPolicy().getModelValues()]
      testHelper.fixture = fixture;
    });

    testHelper.runTests();
  });

  // describe('RBAC', () => {
  //   let toolbarSpy: jasmine.Spy;

    // beforeAll(() => {
    // })

    // beforeEach(() => {
    //   const controllerService = TestBed.get(ControllerService)
    //   toolbarSpy = spyOn(controllerService, 'setToolbarData');
    //   TestingUtility.removeAllPermissions();
    // })

    // it('Admin user', () => {
    //   TestingUtility.setAllPermissions();
    //   fixture.detectChanges();

    //   // create button
    //   expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(1);
    //   // edit button
    //   const actionButtons = fixture.debugElement.queryAll(By.css('.global-table-action-icon'));
    //   expect(actionButtons.length).toBe(2);
    //   expect(actionButtons[0].nativeElement.textContent).toBe('edit');
    //   // delete button
    //   expect(actionButtons[1].nativeElement.textContent).toBe('delete');
    // });

    // it('create access only', () => {
    //   TestingUtility.addPermissions([UIRolePermissions.monitoringalertpolicy_create]);
    //   fixture.detectChanges();

    //   // create button
    //   expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(1);
    //   // edit button not present
    //   const actionButtons = fixture.debugElement.queryAll(By.css('.global-table-action-icon'));
    //   expect(actionButtons.length).toBe(0);
    // });

    // it('edit access only', () => {
    //   TestingUtility.addPermissions([UIRolePermissions.monitoringalertpolicy_update]);
    //   fixture.detectChanges();

    //   // create button
    //   expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(0);
    //   // edit button not present
    //   const actionButtons = fixture.debugElement.queryAll(By.css('.global-table-action-icon'));
    //   expect(actionButtons.length).toBe(1);
    //   expect(actionButtons[0].nativeElement.textContent).toBe('edit');
    // });

    // it('delete access only', () => {
    //   TestingUtility.addPermissions([UIRolePermissions.monitoringalertpolicy_delete]);
    //   fixture.detectChanges();

    //   // create button
    //   expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(0);
    //   // edit button not present
    //   const actionButtons = fixture.debugElement.queryAll(By.css('.global-table-action-icon'));
    //   expect(actionButtons.length).toBe(1);
    //   expect(actionButtons[0].nativeElement.textContent).toBe('delete');
    // });

  // });

});
