import { Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { AbstractControl, FormControl, FormGroupDirective, NgForm, ValidatorFn } from '@angular/forms';
import { ErrorStateMatcher } from '@angular/material/core';
import { Utility } from '@app/common/Utility';
import { BehaviorSubject } from 'rxjs/BehaviorSubject';
import { Observable } from 'rxjs/Observable';
import { map } from 'rxjs/operators/map';
import { startWith } from 'rxjs/operators/startWith';

/** Error when invalid control is touched, or submitted.
 * Modified from https://material.angular.io/components/input/examples
 */
class ObservableErrorStateMatcher extends ErrorStateMatcher {
  private observer = new BehaviorSubject<any>({ errorState: false, controlState: 'PENDING' });
  isErrorState(control: FormControl | null, form: FormGroupDirective | NgForm | null): boolean {
    const errorState = super.isErrorState(control, form);
    this.observer.next(errorState);
    return errorState;
  }

  errorStateObserver(): Observable<any> {
    return this.observer;
  }
}

interface LabelSelectorMetadata {
  formControl: FormControl;
  filteredLabels: Observable<string[]>;
  errorStateMatcher: ErrorStateMatcher;
  id: number;
}

@Component({
  selector: 'app-labelselector',
  templateUrl: './labelselector.component.html',
  styleUrls: ['./labelselector.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class LabelselectorComponent implements OnInit {
  labelSelectorIds: Array<LabelSelectorMetadata> = [];
  nextId = 0;
  topLabels: Array<string> = [];

  @Input() labels: any = {};
  @Input() namespace = 'label';

  @Output() selectedLabels: any;
  @Output() validationObservable: EventEmitter<any> = new EventEmitter(true);
  private observable: BehaviorSubject<any> = new BehaviorSubject<any>({ status: 'VALID' });

  constructor() { }

  ngOnInit() {
    this.topLabels = Object.keys(this.labels).map(s => s + ':');
    this.generateNewLabel();
    this.validationObservable.emit(this.observable);
  }

  generateNewLabel() {
    const formControl = new FormControl('', [
      this.checkValidLabel()
    ]);
    const filteredLabels = formControl.valueChanges.pipe(
      startWith(''),
      map(val => this.filterLabel(val))
    );
    const id = this.nextId;
    this.nextId++;
    const errorStateMatcher = new ObservableErrorStateMatcher();
    this.addErrorObserver(errorStateMatcher.errorStateObserver());
    this.labelSelectorIds.push({ formControl: formControl, filteredLabels: filteredLabels, id: id, errorStateMatcher: errorStateMatcher });
  }

  addErrorObserver(observer: Observable<any>) {
    observer.subscribe(
      (errorState) => {
        let status = '';
        if (errorState || !this.allLabelsValid()) {
          status = 'INVALID';
        } else {
          status = 'VALID';
        }
        // Only send if its a new value
        if (this.observable.getValue().status !== status) {
          this.observable.next({ status: status });
        }
      }
    );
  }

  checkValidLabel(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } => {
      const inputs = control.value.split(':');
      const isValid = (inputs.length === 2 && inputs[0].trim() !== '' && inputs[1].trim() !== '') || control.value === '';
      return (!isValid) ? { 'invalidLabel': { value: control.value } } : null;
    };
  }

  /**
   * We filter which suggestions to show in the autocomplete.
   * Once the key is matched, we begin showing the possible values for that key.
   */
  filterLabel(val: string): string[] {
    if (val.split(':').length > 1) {
      const labels = val.split(':');
      const object = labels[0].toLowerCase().replace(/\s/g, '');
      if (this.labels[object] != null) {
        const options = this.labels[object].map(s => object + ': ' + s);
        return options.filter(option => option.toLowerCase().replace(/\s/g, '').indexOf(val.toLowerCase().replace(/\s/g, '')) === 0);
      }
    } else {
      return this.topLabels.filter(option => option.toLowerCase().replace(/\s/g, '').indexOf(val.toLowerCase().replace(/\s/g, '')) === 0);
    }
  }

  allLabelsValid(): boolean {
    for (const labelObj of this.labelSelectorIds) {
      if (labelObj.formControl.status === 'INVALID') {
        return false;
      }
    }
    return true;
  }

  hasEmptyLabels(): boolean {
    for (const labelObj of this.labelSelectorIds) {
      if (labelObj.formControl.value === '') {
        return true;
      }
    }
    return false;
  }

  addNewLabel() {
    // If there is an existing label that is blank or invalid, don't add a new one yet.
    if (this.allLabelsValid() && !this.hasEmptyLabels()) {
      this.generateNewLabel();
    }
  }

  removeLabel(id) {
    // Can't delete if there is only one label.
    // However, we should clear any entry that exists
    if (this.labelSelectorIds.length === 1) {
      this.labelSelectorIds[0].formControl.setValue('');
      return;
    }
    const _ = Utility.getLodash();
    _.remove(this.labelSelectorIds, (x) => x.id === id);
  }

}
