import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { SyslogComponent } from './syslog.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { PrimengModule } from '@app/lib/primeng.module';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MessageService } from '@app/services/message.service';
import { RouterTestingModule } from '@angular/router/testing';
import { AuthService } from '@app/services/auth.service';

describe('SyslogComponent', () => {
  let component: SyslogComponent;
  let fixture: ComponentFixture<SyslogComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [SyslogComponent],
      imports: [
        MaterialdesignModule,
        RouterTestingModule,
        FormsModule,
        ReactiveFormsModule,
        PrimengModule,
        NoopAnimationsModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        UIConfigsService,
        AuthService,
        MessageService,
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(SyslogComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
