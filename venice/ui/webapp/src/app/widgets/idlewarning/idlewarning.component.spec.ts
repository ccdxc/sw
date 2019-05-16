/* ---------------------------------------------------
    Angular JS libraries
----------------------------------------------------- */
import {  TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { RouterTestingModule } from '@angular/router/testing';
/* ---------------------------------------------------
    Venice App libraries
----------------------------------------------------- */
import { IdleWarningComponent } from './idlewarning.component';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { MatIconRegistry } from '@angular/material';
/* ---------------------------------------------------
    Third-party libraries
----------------------------------------------------- */
import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { MatDialog } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

import { NgModule } from '@angular/core';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from '@app/services/message.service';


@NgModule({
  declarations: [IdleWarningComponent],
  entryComponents: [IdleWarningComponent],
  exports: [IdleWarningComponent],
})
class TestModule { }

describe('IdleWarningComponent', () => {
  let component: IdleWarningComponent;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      imports: [
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        NoopAnimationsModule,
        TestModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MatDialog,
        MessageService
      ]
    });
      });

  beforeEach(() => {
    // fixture = TestBed.createComponent(IdleWarningComponent);
    const dialog = TestBed.get(MatDialog);
    const componentRef = dialog.open(IdleWarningComponent, {
      data: { countdown: 0 }
    });
    component = componentRef.componentInstance;
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
