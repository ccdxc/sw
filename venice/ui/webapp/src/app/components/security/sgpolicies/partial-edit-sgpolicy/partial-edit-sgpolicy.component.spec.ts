import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PartialEditSgpolicyComponent } from './partial-edit-sgpolicy.component';
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
import { SecurityService } from '@app/services/generated/security.service';
import { AuthService } from '@app/services/auth.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';

import { NewsgpolicyComponent } from '../newsgpolicy/newsgpolicy.component';
import { WorkloadService } from '@app/services/generated/workload.service';

describe('PartialEditSgpolicyComponent', () => {
  let component: PartialEditSgpolicyComponent;
  let fixture: ComponentFixture<PartialEditSgpolicyComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ PartialEditSgpolicyComponent ],
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
        MaterialdesignModule
        ],
      providers: [
        ControllerService,
        UIConfigsService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        SecurityService,
        MessageService,
        WorkloadService
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PartialEditSgpolicyComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
