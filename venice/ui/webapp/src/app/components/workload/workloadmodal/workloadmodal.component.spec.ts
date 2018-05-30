/* ---------------------------------------------------
    Angular JS libraries
----------------------------------------------------- */
import { async, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
/* ---------------------------------------------------
    Venice App libraries
----------------------------------------------------- */
import { WorkloadModalComponent } from './workloadmodal.component';
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
import { SharedModule } from '@app/components/shared//shared.module';
import { LabelselectorComponent } from '@app/components/workload/labelselector/labelselector.component';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { CommonModule } from '@angular/common';

@NgModule({
  declarations: [WorkloadModalComponent, LabelselectorComponent],
  imports: [CommonModule, SharedModule, MaterialdesignModule, FormsModule, ReactiveFormsModule],
  entryComponents: [WorkloadModalComponent, LabelselectorComponent],
  exports: [WorkloadModalComponent, LabelselectorComponent],
})
class TestModule { }

describe('WorkloadModalComponent', () => {
  let component: WorkloadModalComponent;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      imports: [
        CommonModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        SharedModule,
        NoopAnimationsModule,
        TestModule,
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
    const dialog = TestBed.get(MatDialog);
    const componentRef = dialog.open(WorkloadModalComponent, {
      data: {
        securityGroups: ['SG1', 'SG2'],
        selectedWorkloads: [],
        labels: { 'Loc': ['NL', 'AMS'], 'Env': ['test', 'prod'] }
      }
    });
    component = componentRef.componentInstance;
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('filter security groups', () => {
    expect(component.filterSecurityGroups('S').length).toBe(2);
    expect(component.filterSecurityGroups('SG1').length).toBe(1);
    expect(component.filterSecurityGroups('SG3').length).toBe(0);
  });

  it('form valid', () => {
    // Testing new security group form
    component.newSecurityGroup = 'new';
    component.newSecurityGroupFormControl.setValue('SG1');
    expect(component.formValid()).toBeFalsy();
    component.newSecurityGroupFormControl.setValue('SG3');
    expect(component.formValid()).toBeFalsy();
    component.notEmptyFormControl.setValue('Description');
    expect(component.formValid()).toBeTruthy();
    component.labelSelectorValidationStatus = 'INVALID';
    expect(component.formValid()).toBeFalsy();

    // Testing existing security group form
    component.newSecurityGroup = 'existing';
    component.existingSecurityGroupFormControl.setValue('SG3');
    expect(component.formValid()).toBeFalsy();
    component.existingSecurityGroupFormControl.setValue('SG1');
    expect(component.formValid()).toBeTruthy();
  });
});
