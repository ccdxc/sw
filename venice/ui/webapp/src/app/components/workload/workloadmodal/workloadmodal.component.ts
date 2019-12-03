import { Component, Inject, OnInit, ViewEncapsulation } from '@angular/core';
import {BaseComponent} from '@components/base/base.component';
import { AbstractControl, FormControl, FormGroupDirective, NgForm, ValidatorFn, Validators } from '@angular/forms';
import { MAT_DIALOG_DATA, MatDialogRef } from '@angular/material';
import { ErrorStateMatcher } from '@angular/material/core';
import { Utility } from '@app/common/Utility';
import { BehaviorSubject, Observable } from 'rxjs';
import { map, startWith } from 'rxjs/operators';
import {ControllerService} from '@app/services/controller.service';

/** Error when invalid control is touched, or submitted.
 * On first focus, the error state won't activate till the user leaves the field
 * After the first enter, anytime the user reenters, it will go to default error
 * state matching, which is instantaneous checking.
 * Modified from https://material.angular.io/components/input/examples
 */
class ObservableInstantErrorStateMatcher implements ErrorStateMatcher {
  private observer = new BehaviorSubject<any>({ status: '' });
  isErrorState(control: FormControl | null, form: FormGroupDirective | NgForm | null): boolean {
    const isSubmitted = form && form.submitted;
    const val = !!(control && control.invalid && (control.touched || isSubmitted));

    this.observer.next({ status: this.computeStatus(val, control.status) });
    return val;
  }

  computeStatus(errorState: boolean, controlStatus: string): string {
    let status = '';
    if (errorState) {
      status = 'INVALID';
    } else if (controlStatus === 'VALID') {
      status = 'VALID';
    }
    return status;
  }

  errorStateObserver(): Observable<any> {
    return this.observer;
  }
}

@Component({
  selector: 'app-workloadmodal',
  templateUrl: './workloadmodal.component.html',
  styleUrls: ['./workloadmodal.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class WorkloadModalComponent extends BaseComponent implements OnInit {
  icon: any = {
    url: 'security-policy'
  };

  // Error messages
  emptyFieldErrorMessage = 'This field cannot be blank.';
  notExistingSecurityGroupErrorMessage = 'Please choose an existing Security Group.';

  labelSelectorValidationStatus = 'VALID'; // Default value is valid because its allowed to not have a label

  newSecurityGroup = 'new';
  errorStateMatcherName = new ObservableInstantErrorStateMatcher();
  errorStateMatcherDescription = new ObservableInstantErrorStateMatcher();
  errorStateMatcherExisting = new ObservableInstantErrorStateMatcher();

  newSecurityGroupFormControl: FormControl = new FormControl('', [
    Validators.required,
    this.checkExistingName(false)
  ]);

  notEmptyFormControl: FormControl = new FormControl('', [
    Validators.required,
  ]);

  existingSecurityGroupFormControl: FormControl = new FormControl('', [
    Validators.required,
    this.checkExistingName(true)
  ]);

  filteredOptions: any;

  // without controllerService, icon register won't work
  constructor(
    protected controllerService: ControllerService,
    public dialogRef: MatDialogRef<WorkloadModalComponent>,
    @Inject(MAT_DIALOG_DATA) public data: any) {
    super(controllerService);
  }

  ngOnInit() {
    // filtering for the existing security group option
    this.filteredOptions = this.existingSecurityGroupFormControl.valueChanges.pipe(
      startWith(''),
      map(val => this.filterSecurityGroups(val))
    );
  }

  formValid() {
    if (this.newSecurityGroup === 'new') {
      return this.newSecurityGroupFormControl.valid
        && this.notEmptyFormControl.valid
        && this.labelSelectorValidationStatus === 'VALID';
    } else if (this.newSecurityGroup === 'existing') {
      return this.existingSecurityGroupFormControl.valid;
    }
    return false;
  }

  filterSecurityGroups(val: string): string[] {
    return this.data.securityGroups.filter(option => option.toLowerCase().indexOf(val.toLowerCase()) === 0);
  }

  onNoClick(): void {
    this.dialogRef.close();
  }

  submitClick(): void {
    if (!this.formValid()) {
      return;
    }
    if (this.newSecurityGroup === 'new') {
      console.log('creating new group', this.newSecurityGroupFormControl.value, this.notEmptyFormControl.value);
    } else {
      console.log('joining existing group', this.existingSecurityGroupFormControl.value);
    }
    this.dialogRef.close();
  }

  checkExistingName(shouldExist: boolean): ValidatorFn {
    const _ = Utility.getLodash();
    return (control: AbstractControl): { [key: string]: any } => {
      const contains = _.includes(this.data.securityGroups, control.value);
      if (shouldExist) {
        return (!contains) ? { 'notExistingName': { value: control.value } } : null;
      } else {
        return contains ? { 'takenName': { value: control.value } } : null;
      }
    };
  }

  setValidationObservable($event) {
    $event.subscribe((data) => {
      // Timeout is needed to prevent the tree from changing during the time
      // angular is updating the model. Currently unable to create a better solution
      setTimeout(() => { this.labelSelectorValidationStatus = data.status; }, 0);
    });
  }
}
