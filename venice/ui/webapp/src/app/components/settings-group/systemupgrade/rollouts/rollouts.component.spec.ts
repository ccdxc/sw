import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { RolloutsComponent } from './rollouts.component';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import {NoopAnimationsModule} from '@angular/platform-browser/animations';
import {HttpClientTestingModule} from '@angular/common/http/testing';
import {PrimengModule} from '@lib/primeng.module';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {RouterTestingModule} from '@angular/router/testing';
import {SharedModule} from '@components/shared/shared.module';
import {ControllerService} from '@app/services/controller.service';
import {ConfirmationService} from 'primeng/api';
import {LogService} from '@app/services/logging/log.service';
import {LogPublishersService} from '@app/services/logging/log-publishers.service';
import {MatIconRegistry} from '@angular/material';
import {AuthService} from '@app/services/generated/auth.service';
import {MessageService} from '@app/services/message.service';
import {StagingService} from '@app/services/generated/staging.service';
import {RolloutService} from '@app/services/generated/rollout.service';

describe('RolloutsComponent', () => {
  let component: RolloutsComponent;
  let fixture: ComponentFixture<RolloutsComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ RolloutsComponent ],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        SharedModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        RolloutService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        AuthService,
        MessageService,
        StagingService
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(RolloutsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
