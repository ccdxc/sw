/**-----
 Angular imports
 ------------------*/
 import { async, ComponentFixture, TestBed } from '@angular/core/testing';
 import { HttpClientTestingModule } from '@angular/common/http/testing';
 import { Component } from '@angular/core';
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
import { LicenseService } from '@app/services/license.service';
 import { EventsService } from '@app/services/events.service';
 import { SearchService } from '@app/services/generated/search.service';
 import { MonitoringService } from '@app/services/generated/monitoring.service';
 import { MessageService } from '@app/services/message.service';
 import { WidgetsModule } from 'web-app-framework';

 import { AuthService } from '@app/services/auth.service';
 import { ClusterService } from '@app/services/generated/cluster.service';


import { DscprofilesComponent } from './dscprofiles.component';
import { NewdscprofileComponent } from './newdscprofile/newdscprofile.component';

describe('DscprofileComponent', () => {
  let component: DscprofilesComponent;
  let fixture: ComponentFixture<DscprofilesComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ DscprofilesComponent, NewdscprofileComponent ],
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
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        UIConfigsService,
        LicenseService,
        AuthService,
        EventsService,
        SearchService,
        MonitoringService,
        MessageService,
        ClusterService
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DscprofilesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
