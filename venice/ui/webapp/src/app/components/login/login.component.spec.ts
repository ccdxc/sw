import { HttpClientTestingModule } from '@angular/common/http/testing';
import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { FormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';
import { CoreModule } from '@app/core';

import { AuthService } from '../../services/auth.service';
import { AuthService as AuthServiceGen } from '../../services/generated/auth.service';
import { ControllerService } from '../../services/controller.service';
import { LoginComponent } from './login.component';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { ConfirmationService } from 'primeng/primeng';
import { MessageService } from '@app/services/message.service';

describe('LoginComponent', () => {
  let component: LoginComponent;
  let fixture: ComponentFixture<LoginComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [LoginComponent],
      imports: [
        RouterTestingModule,
        FormsModule,
        HttpClientTestingModule,
        CoreModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        AuthService,
        MatIconRegistry,
        UIConfigsService,
        LicenseService,
        MessageService,
        AuthServiceGen
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(LoginComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
