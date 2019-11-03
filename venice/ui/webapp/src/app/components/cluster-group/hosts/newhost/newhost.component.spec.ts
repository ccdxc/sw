import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { NewhostComponent } from './newhost.component';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import {NoopAnimationsModule} from '@angular/platform-browser/animations';
import {HttpClientTestingModule} from '@angular/common/http/testing';
import {PrimengModule} from '@lib/primeng.module';
import {WidgetsModule} from 'web-app-framework';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {RouterTestingModule} from '@angular/router/testing';
import {SharedModule} from '@components/shared/shared.module';
import {ControllerService} from '@app/services/controller.service';
import {ConfirmationService} from 'primeng/api';
import {LogService} from '@app/services/logging/log.service';
import {LogPublishersService} from '@app/services/logging/log-publishers.service';
import {MatIconRegistry} from '@angular/material';
import {MessageService} from '@app/services/message.service';
import {ClusterService} from '@app/services/generated/cluster.service';
import {BehaviorSubject} from 'rxjs';
import {By} from '@angular/platform-browser';
import {RouterLinkStubDirective} from '@common/RouterLinkStub.directive.spec';
import {TestingUtility} from '@common/TestingUtility';
import {ClusterHost} from '@sdk/v1/models/generated/cluster';
import {DebugElement} from '@angular/core';
import { AuthService } from '@app/services/auth.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { StagingService } from '@app/services/generated/staging.service';



describe('NewhostComponent', () => {
  let component: NewhostComponent;
  let fixture: ComponentFixture<NewhostComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
     declarations: [NewhostComponent],
     imports: [
       FormsModule,
       ReactiveFormsModule,
       NoopAnimationsModule,
       HttpClientTestingModule,
       PrimengModule,
       WidgetsModule,
       MaterialdesignModule,
       RouterTestingModule,
       SharedModule,
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
       MessageService,
       StagingService
     ]
   });
  });

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NewhostComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NewhostComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
