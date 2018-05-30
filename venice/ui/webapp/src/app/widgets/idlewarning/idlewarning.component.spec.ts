/* ---------------------------------------------------
    Angular JS libraries
----------------------------------------------------- */
import { async, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
/* ---------------------------------------------------
    Venice App libraries
----------------------------------------------------- */
import { IdleWarningComponent } from './idlewarning.component';
import { ControllerService } from '@app/services/controller.service';
import { MatIconRegistry } from '@angular/material';
/* ---------------------------------------------------
    Third-party libraries
----------------------------------------------------- */
import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { MatDialog } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

import { NgModule } from '@angular/core';


@NgModule({
  declarations: [IdleWarningComponent],
  entryComponents: [IdleWarningComponent],
  exports: [IdleWarningComponent],
})
class TestModule { }

describe('IdleWarningComponent', () => {
  let component: IdleWarningComponent;

  beforeEach(async(() => {
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
        MatIconRegistry,
        MatDialog
      ]
    })
      .compileComponents();
  }));

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
