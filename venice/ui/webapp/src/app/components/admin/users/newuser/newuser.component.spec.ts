import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { RouterTestingModule } from '@angular/router/testing';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { PrimengModule } from '@app/lib/primeng.module';
import { WidgetsModule } from 'web-app-framework';
import { SharedModule } from '@app/components/shared/shared.module';
import { AuthService as AuthServiceGen } from '@app/services/generated/auth.service';
import { NewuserComponent } from './newuser.component';
import { NewroleComponent } from '../newrole/newrole.component';
import { NewrolebindingComponent } from '../newrolebinding/newrolebinding.component';
import { UsersComponent } from '../users.component';
import { MessageService } from '@app/services/message.service';
import { StagingService } from '@app/services/generated/staging.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService } from '@app/services/auth.service';

describe('NewuserComponent', () => {
  let component: NewuserComponent;
  let fixture: ComponentFixture<NewuserComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [NewuserComponent, UsersComponent, NewroleComponent, NewrolebindingComponent],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        PrimengModule,
        WidgetsModule,
        MaterialdesignModule,
        RouterTestingModule,
        SharedModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        LicenseService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        AuthServiceGen,
        MessageService,
        StagingService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(NewuserComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
