/**-----
 Angular imports
 ------------------*/
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { Component } from '@angular/core';
import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { SharedModule } from '@app/components/shared/shared.module';
import { AlerttableService } from '@app/services/alerttable.service';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { AlertseventspageComponent } from './alertseventspage.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { EventsService } from '@app/services/events.service';
import { SearchService } from '@app/services/generated/search.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { MessageService } from '@app/services/message.service';
import { TestingUtility } from '@app/common/TestingUtility';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { AuthService } from '@app/services/auth.service';


@Component({
  template: ''
})
class DummyComponent { }

describe('AlertseventspageComponent', () => {
  let component: AlertseventspageComponent;
  let fixture: ComponentFixture<AlertseventspageComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [AlertseventspageComponent, DummyComponent],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        AlerttableService,
        MatIconRegistry,
        UIConfigsService,
        LicenseService,
        EventsService,
        SearchService,
        MonitoringService,
        MessageService,
        AuthService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(AlertseventspageComponent);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    fixture.detectChanges();
    expect(component).toBeTruthy();
  });

  describe('RBAC', () => {
      let toolbarSpy: jasmine.Spy;

      beforeEach(() => {
        TestingUtility.removeAllPermissions();
        const controllerService = TestBed.get(ControllerService);
        toolbarSpy = spyOn(controllerService, 'setToolbarData');
      });

    it('Admin user', () => {
      TestingUtility.setAllPermissions();
      fixture.detectChanges();

      // create button
      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(1);
      expect(toolbarSpy.calls.mostRecent().args[0].buttons[0].text).toBe('ALERT POLICIES');

      // Switch tab
      component.setTabToolbar('events');
      fixture.detectChanges();

      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(2);
      expect(toolbarSpy.calls.mostRecent().args[0].buttons[0].text).toBe('EXPORT EVENTS');
      expect(toolbarSpy.calls.mostRecent().args[0].buttons[1].text).toBe('EVENT POLICIES');
    });

    it('alert policy only', () => {
      TestingUtility.addPermissions([UIRolePermissions.monitoringalertpolicy_read]);
      fixture.detectChanges();

      // create button
      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(1);

      // Switch tab
      component.setTabToolbar('events');
      fixture.detectChanges();

      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(0);
    });

    it('event policy only', () => {
      TestingUtility.addPermissions([UIRolePermissions.monitoringeventpolicy_read]);
      fixture.detectChanges();

      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(0);

      // Switch tab
      component.setTabToolbar('events');
      fixture.detectChanges();

      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(1);
      expect(toolbarSpy.calls.mostRecent().args[0].buttons[0].text).toBe('EVENT POLICIES');
    });

    it('no access', () => {
      fixture.detectChanges();

      // create button
      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(0);
      // Switch tab
      component.setTabToolbar('events');
      fixture.detectChanges();

      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(0);
    });
  });
});
