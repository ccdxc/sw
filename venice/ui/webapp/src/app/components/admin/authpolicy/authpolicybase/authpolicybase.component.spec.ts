import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { SharedModule } from '@app/components/shared/shared.module';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';

import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { MessageService } from '@app/services/message.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { AuthService } from '@app/services/auth.service';

import { AuthpolicybaseComponent } from './authpolicybase.component';

describe('AuthpolicybaseComponent', () => {
  let component: AuthpolicybaseComponent;
  let fixture: ComponentFixture<AuthpolicybaseComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [AuthpolicybaseComponent],
      imports: [
        MaterialdesignModule,
        RouterTestingModule,
        HttpClientTestingModule,
        SharedModule,
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        PrimengModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService,
        UIConfigsService,
        LicenseService,
        AuthService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(AuthpolicybaseComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should lower rank', () => {
    const spy = spyOn(component.changeAuthRank, 'emit');
    component.currentRank = 0;
    component.numRanks = 2;
    expect(component.canLowerAuthRank()).toBeTruthy();
    expect(component.canRaiseAuthRank()).toBeFalsy();
    component.raiseAuthRank();
    expect(spy).toHaveBeenCalledTimes(0);
    component.lowerAuthRank();
    expect(spy).toHaveBeenCalledTimes(1);
    expect(spy).toHaveBeenCalledWith(1);
  });

  it('should raise rank', () => {
    const spy = spyOn(component.changeAuthRank, 'emit');
    component.currentRank = 1;
    component.numRanks = 2;
    expect(component.canLowerAuthRank()).toBeFalsy();
    expect(component.canRaiseAuthRank()).toBeTruthy();
    component.lowerAuthRank();
    expect(spy).toHaveBeenCalledTimes(0);
    component.raiseAuthRank();
    expect(spy).toHaveBeenCalledTimes(1);
    expect(spy).toHaveBeenCalledWith(0);
  });

  it('should handle missing input gracefully', () => {
    const spy = spyOn(component.changeAuthRank, 'emit');
    component.numRanks = 2;
    expect(component.canLowerAuthRank()).toBeFalsy();
    expect(component.canRaiseAuthRank()).toBeFalsy();
    component.raiseAuthRank();
    expect(spy).toHaveBeenCalledTimes(0);
    component.lowerAuthRank();
    expect(spy).toHaveBeenCalledTimes(0);

    component.currentRank = 1;
    component.numRanks = null;
    expect(component.canLowerAuthRank()).toBeFalsy();
    expect(component.canRaiseAuthRank()).toBeFalsy();
    component.raiseAuthRank();
    expect(spy).toHaveBeenCalledTimes(0);
    component.lowerAuthRank();
    expect(spy).toHaveBeenCalledTimes(0);
  });
});
