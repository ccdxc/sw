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
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { EventsService } from '@app/services/events.service';
import { SearchService } from '@app/services/generated/search.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MessageService } from '@app/services/message.service';
import { WidgetsModule } from 'web-app-framework';

import { NewtechsupportComponent } from './newtechsupport/newtechsupport.component';
import { TechsupportComponent } from './techsupport.component';
import { AuthService } from '@app/services/auth.service';
import { TestTablevieweditRBAC } from '@app/components/shared/tableviewedit/tableviewedit.component.spec';
import { MonitoringTechSupportRequest } from '@sdk/v1/models/generated/monitoring';
import { TestingUtility } from '@app/common/TestingUtility';

describe('TechsupportComponent', () => {
  let component: TechsupportComponent;
  let fixture: ComponentFixture<TechsupportComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [TechsupportComponent, NewtechsupportComponent],
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
        ClusterService,
        UIConfigsService,
        AuthService,
        EventsService,
        SearchService,
        MonitoringService,
        MessageService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(TechsupportComponent);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    fixture.detectChanges();
    expect(component).toBeTruthy();
  });

  describe('RBAC', () => {
    const testHelper = new TestTablevieweditRBAC('monitoringtechsupportrequest', ['clusternode_read', 'clusterdistributedservicecard_read']);
    testHelper.skipEdit = true;

    beforeEach(() => {
      const service = TestBed.get(MonitoringService);
      spyOn(service, 'WatchTechSupportRequest').and.returnValue(
        TestingUtility.createWatchEventsSubject([new MonitoringTechSupportRequest()])
      );
      testHelper.fixture = fixture;
    });

    testHelper.runTests();
  });

});
