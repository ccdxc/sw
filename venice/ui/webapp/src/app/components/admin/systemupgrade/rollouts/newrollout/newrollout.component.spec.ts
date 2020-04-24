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
import { LicenseService } from '@app/services/license.service';
 import { EventsService } from '@app/services/events.service';
 import { SearchService } from '@app/services/generated/search.service';
 import { MessageService } from '@app/services/message.service';
 import { WidgetsModule } from 'web-app-framework';

import { NewrolloutComponent } from './newrollout.component';
import { RolloutService } from '@app/services/generated/rollout.service';
import { ObjstoreService } from '@app/services/generated/objstore.service';
import { AuthService } from '@app/services/auth.service';
import { ClusterService } from '@app/services/generated/cluster.service';

describe('NewrolloutComponent', () => {
  let component: NewrolloutComponent;
  let fixture: ComponentFixture<NewrolloutComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [ NewrolloutComponent ],
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
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        UIConfigsService,
        LicenseService,
        AuthService,
        EventsService,
        SearchService,
        MessageService,
        RolloutService,
        ObjstoreService,
        ClusterService
      ]
    });
    });

  beforeEach(() => {
    fixture = TestBed.createComponent(NewrolloutComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
