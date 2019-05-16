/**-----
 Angular imports
 ------------------*/
 import { HttpClientTestingModule } from '@angular/common/http/testing';
import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { SharedModule } from '@app/components/shared/shared.module';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MessageService } from '@app/services/message.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { ConfirmationService } from 'primeng/primeng';
import { WidgetsModule } from 'web-app-framework';
import { MonitoringGroupModule } from '../monitoring-group.module';
import { FwlogpoliciesComponent } from './fwlogpolicies/fwlogpolicies.component';
import { NewfwlogpolicyComponent } from './fwlogpolicies/newfwlogpolicy/newfwlogpolicy.component';
import { FwlogsComponent } from './fwlogs.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { AuthService } from '@app/services/auth.service';
import { TestingUtility } from '@app/common/TestingUtility';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TelemetryqueryService } from '@app/services/generated/telemetryquery.service';


describe('fwlogsComponent', () => {
   let component: FwlogsComponent;
   let fixture: ComponentFixture<FwlogsComponent>;

    configureTestSuite(() => {
    TestBed.configureTestingModule({
       declarations: [
         FwlogsComponent,
         FwlogpoliciesComponent,
         NewfwlogpolicyComponent,
       ],
       imports: [
         FormsModule,
         ReactiveFormsModule,
         NoopAnimationsModule,
         SharedModule,
         HttpClientTestingModule,
         PrimengModule,
         MaterialdesignModule,
         WidgetsModule,
         RouterTestingModule,
         MonitoringGroupModule
       ],
       providers: [
         ControllerService,
         UIConfigsService,
         AuthService,
         TelemetryqueryService,
         ConfirmationService,
         LogService,
         LogPublishersService,
         MonitoringService,
         MatIconRegistry,
         MessageService
       ]
     });
    });

   beforeEach(() => {
     fixture = TestBed.createComponent(FwlogsComponent);
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

      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(3);
    });

    it('fwlogs policy only', () => {
      TestingUtility.addPermissions([UIRolePermissions.monitoringfwlogpolicy_read]);
      fixture.detectChanges();

      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(3);
    });

    it('no access', () => {
      fixture.detectChanges();

      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(2);
    });
  });
 });

