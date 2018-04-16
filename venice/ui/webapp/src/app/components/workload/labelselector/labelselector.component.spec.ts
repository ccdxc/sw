import { CommonModule } from '@angular/common';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { ReactiveFormsModule, FormControl, Validators } from '@angular/forms';
import { MaterialdesignModule } from '@lib/materialdesign.module';

import { LabelselectorComponent } from './labelselector.component';

describe('LabelselectorComponent', () => {
  let component: LabelselectorComponent;
  let fixture: ComponentFixture<LabelselectorComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ LabelselectorComponent ],
      imports: [CommonModule,  MaterialdesignModule, ReactiveFormsModule]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(LabelselectorComponent);
    component = fixture.componentInstance;
    component.labels = {a: ['1', '2'], b: ['3', '4'], abc: ['123', '122']};
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('check validLabel', () => {
    const formControl = new FormControl;
    const validator = component.checkValidLabel();

    // Happy path
    formControl.setValue('key: value');
    expect(validator(formControl)).toBeNull();

    // no colon separator
    formControl.setValue('key value');
    expect(validator(formControl)).toBeDefined();

    // no value
    formControl.setValue('key:     ');
    expect(validator(formControl)).toBeDefined();

    // no key
    formControl.setValue('      value');
    expect(validator(formControl)).toBeDefined();

    // no input - should be truthy so that the initial state
    // of the item is green if they want no labels on a workload
    formControl.setValue('');
    expect(validator(formControl)).toBeNull();

    // blank input
    formControl.setValue('          ');
    expect(validator(formControl)).toBeDefined();

  });

  it('check filterLabel', () => {
    let search = 'a';
    let ret = component.filterLabel(search);
    expect(ret.length).toBe(2);

    // the colon should make it switch to suggesting values
    search = 'a:';
    ret = component.filterLabel(search);
    expect(ret.length).toBe(2);
    expect(ret[0]).toBe('a: 1');

    // should ignore whitespace and case
    search = '   A    :    ';
    ret = component.filterLabel(search);
    expect(ret.length).toBe(2);
    expect(ret[0]).toBe('a: 1');

    // No label matches the key
    search = 'c';
    ret = component.filterLabel(search);
    expect(ret.length).toBe(0);

    // no label matches the value
    search = 'a: 5';
    ret = component.filterLabel(search);
    expect(ret.length).toBe(0);

    //
    search = 'abc: 1';
    ret = component.filterLabel(search);
    expect(ret.length).toBe(2);

    search = 'abc: 12';
    ret = component.filterLabel(search);
    expect(ret.length).toBe(2);

    search = 'abc: 123';
    ret = component.filterLabel(search);
    expect(ret.length).toBe(1);
  });

  it('check allLabelsValid', () => {
    const labelSelector1 = {
      formControl: new FormControl('', component.checkValidLabel()),
      id: 1,
      filteredLabels: null,
      errorStateMatcher: null
    };
    const labelSelector2 = {
      formControl: new FormControl('', component.checkValidLabel()),
      id: 2,
      filteredLabels: null,
      errorStateMatcher: null
    };
    const labelSelector3 = {
      formControl: new FormControl('', component.checkValidLabel()),
      id: 3,
      filteredLabels: null,
      errorStateMatcher: null
    };
    component.labelSelectorIds = [labelSelector1, labelSelector2, labelSelector3];

    // currently all blank, so all are valid
    expect(component.allLabelsValid()).toBeTruthy();

    // one form invalid
    labelSelector1.formControl.setValue('b: 3');
    labelSelector2.formControl.setValue('a: 1');
    labelSelector3.formControl.setValue('asdf');
    expect(component.allLabelsValid()).toBeFalsy();

    // no blank - should be valid
    labelSelector3.formControl.setValue('abc: 123');
    expect(component.allLabelsValid()).toBeTruthy();
  });

  it('add and remove of labels', () => {
    const labelSelector1 = {
      formControl: new FormControl('', component.checkValidLabel()),
      id: 1,
      filteredLabels: null,
      errorStateMatcher: null
    };
    const labelSelector2 = {
      formControl: new FormControl('', component.checkValidLabel()),
      id: 2,
      filteredLabels: null,
      errorStateMatcher: null
    };
    const labelSelector3 = {
      formControl: new FormControl('', component.checkValidLabel()),
      id: 3,
      filteredLabels: null,
      errorStateMatcher: null
    };
    component.labelSelectorIds = [labelSelector1, labelSelector2, labelSelector3];

    // there are empty labels, can't create a new label
    component.addNewLabel();
    expect(component.labelSelectorIds.length).toBe(3);

    // attempt to delete all labels. The last one should fail but empty out its contents
    labelSelector1.formControl.setValue('asdf');
    component.removeLabel(3);
    expect(component.labelSelectorIds.length).toBe(2);
    component.removeLabel(2);
    expect(component.labelSelectorIds.length).toBe(1);
    component.removeLabel(1);
    expect(component.labelSelectorIds.length).toBe(1);
    expect(labelSelector1.formControl.value).toBe('');

    // We fill the only lable with text, and add another label
    // first attempt should fail since value isnt valid text
    labelSelector1.formControl.setValue('asdf');
    component.addNewLabel();
    expect(component.labelSelectorIds.length).toBe(1);
    labelSelector1.formControl.setValue('a:1');
    component.addNewLabel();
    expect(component.labelSelectorIds.length).toBe(2);

  });
});
