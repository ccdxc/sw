import { ComponentFixture, TestBed, async } from '@angular/core/testing';
import { FormArray, FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { By } from '@angular/platform-browser';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { MaterialModule } from '../../../material.module';
import { PrimengModule } from '../../../primeng.module';
import { LabelDirective } from '../directives/label.directive';
import { ValueType } from './index';
import { RepeaterComponent } from './repeater.component';
import {DebugElement} from '@angular/core';

function sendClick(elem: DebugElement, fixture: ComponentFixture<RepeaterComponent>) {
  const elemNative = elem.nativeElement;
  elemNative.click();
  fixture.detectChanges();
  return fixture.whenStable();
}
function sendBlur(elem: DebugElement, fixture: ComponentFixture<RepeaterComponent>) {
  const elemNative = elem.nativeElement;
  elemNative.dispatchEvent(new Event('blur'));
  fixture.detectChanges();
  return fixture.whenStable();
}


describe('RepeaterComponent', () => {
  let component: RepeaterComponent;
  let fixture: ComponentFixture<RepeaterComponent>;
  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [RepeaterComponent, LabelDirective],
      imports: [FormsModule, ReactiveFormsModule, PrimengModule, MaterialModule, NoopAnimationsModule]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(RepeaterComponent);
    component = fixture.componentInstance;
    component.data = [
      {
        key: { label: 'severity', value: 'severity' },
        operators: [
          { label: 'is', value: 'is' },
          { label: 'is', value: 'not' }
        ],
        values: [
          { label: 'critical', value: 'critical' },
          { label: 'warning', value: 'warning' },
          { label: 'info', value: 'info' },
        ],
        valueType: ValueType.singleSelect
      },
      {
        key: { label: 'kind', value: 'kind' },
        operators: [
          { label: 'is', value: 'is' },
          { label: 'is not', value: 'is not' },
        ],
        values: [
          { label: 'Node', value: 'Node' },
          { label: 'Network', value: 'Network' },
          { label: 'Endpoint', value: 'Endpoint' },
        ],
        valueType: ValueType.multiSelect
      },
      {
        key: { label: 'name', value: 'name' },
        operators: [
          { label: 'is', value: 'is' },
          { label: 'is not', value: 'is not' },
        ],
        valueType: ValueType.inputField
      },
    ];
  });

  // It is important to clean up after each test
  afterEach(() => {
    TestBed.resetTestingModule();
  });

  it('should add and delete repeaters', () => {
    // should emit a default value
    const spy = spyOn(component.repeaterValues, 'emit');
    fixture.detectChanges();
    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith([{ keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' }]);
    expect(component.repeaterList.length).toBe(1);

    // trash icon should not be visible
    let trashElem = fixture.debugElement.queryAll(By.css('.repeater-trash-icon'));
    expect(trashElem.length).toBe(0);
    // Add icon should be visible
    let addElem = fixture.debugElement.queryAll(By.css('.repeater-and'));
    expect(addElem.length).toBe(1);
    spy.calls.reset();

    // adding another repeater value
    addElem[0].nativeElement.querySelector('span').click();
    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith(
      [
        { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' },
        { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' }
      ]);
    expect(component.repeaterList.length).toBe(2);
    spy.calls.reset();
    fixture.detectChanges();

    // one And, two trash cans
    addElem = fixture.debugElement.queryAll(By.css('.repeater-and'));
    trashElem = fixture.debugElement.queryAll(By.css('.repeater-trash-icon'));
    expect(addElem.length).toBe(1);
    expect(trashElem.length).toBe(2);

    // add
    addElem[0].nativeElement.querySelector('span').click();
    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith([
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' },
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' },
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' }
    ]);
    expect(component.repeaterList.length).toBe(3);
    spy.calls.reset();
    fixture.detectChanges();

    // Should still be able to add since add is not unique, three trash cans
    addElem = fixture.debugElement.queryAll(By.css('.repeater-and'));
    trashElem = fixture.debugElement.queryAll(By.css('.repeater-trash-icon'));
    expect(addElem.length).toBe(1);
    expect(trashElem.length).toBe(3);
    spy.calls.reset();
    fixture.detectChanges();

    // add
    addElem[0].nativeElement.querySelector('span').click();
    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith([
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' },
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' },
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' },
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' }
    ]);
    expect(component.repeaterList.length).toBe(4);
    spy.calls.reset();
    fixture.detectChanges();

    addElem = fixture.debugElement.queryAll(By.css('.repeater-and'));
    trashElem = fixture.debugElement.queryAll(By.css('.repeater-trash-icon'));
    expect(addElem.length).toBe(1);
    expect(trashElem.length).toBe(4);
    spy.calls.reset();
    fixture.detectChanges();

    // delete
    trashElem[0].nativeElement.children[0].click();
    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith([
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' },
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' },
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' }
    ]);
    expect(component.repeaterList.length).toBe(3);
    spy.calls.reset();
    fixture.detectChanges();

    // one And, 3 trash cans
    addElem = fixture.debugElement.queryAll(By.css('.repeater-and'));
    trashElem = fixture.debugElement.queryAll(By.css('.repeater-trash-icon'));
    expect(addElem.length).toBe(1);
    expect(trashElem.length).toBe(3);
    spy.calls.reset();
    fixture.detectChanges();

    // delete
    trashElem[0].nativeElement.children[0].click();
    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith([
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' },
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' }
    ]);
    expect(component.repeaterList.length).toBe(2);
    spy.calls.reset();
    fixture.detectChanges();

    // one And, two trash cans
    addElem = fixture.debugElement.queryAll(By.css('.repeater-and'));
    trashElem = fixture.debugElement.queryAll(By.css('.repeater-trash-icon'));
    expect(addElem.length).toBe(1);
    expect(trashElem.length).toBe(2);


    // delete
    trashElem[0].nativeElement.children[0].click();
    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith([
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' },
    ]);
    expect(component.repeaterList.length).toBe(1);
    spy.calls.reset();
    fixture.detectChanges();

    // one And, two trash cans
    addElem = fixture.debugElement.queryAll(By.css('.repeater-and'));
    trashElem = fixture.debugElement.queryAll(By.css('.repeater-trash-icon'));
    expect(component.repeaterList.length).toBe(1);
    expect(addElem.length).toBe(1);
    expect(trashElem.length).toBe(0);
  });

  it('should load form control data and set correct values-1', () => {
    // Create two repeaters
    component.keyFormName = 'keyControl';
    component.operatorFormName = 'operatorControl';
    component.valueFormName = 'valueControl';
    component.formArray = new FormArray([
      new FormControl({ keyControl: 'name', operatorControl: 'is not', valueControl: 'testing' }),
      new FormControl({ keyControl: 'severity', operatorControl: 'is', valueControl: 'critical' })
    ]);
    spyOn(component.repeaterValues, 'emit');
    fixture.detectChanges();

    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith(
      [
        { keyControl: 'name', operatorControl: 'is not', valueControl: 'testing' },
        { keyControl: 'severity', operatorControl: 'is', valueControl: 'critical' },
      ]
    );

    // Changing key
    // Since we have two repeaters, the number of key options should be three since keys are not unique
    const repeaterKey = fixture.debugElement.queryAll(By.css('.repeater-key'));
    expect(repeaterKey.length).toBe(2);
    repeaterKey[1].nativeElement.children[0].children[0].click();
    fixture.detectChanges();
    let options = repeaterKey[1].queryAll(By.css('.ui-dropdown-item'));
    expect(options.length).toBe(3);
    options[1].nativeElement.click(); // selecting second one
    fixture.detectChanges();

    expect(component.repeaterValues.emit).toHaveBeenCalledTimes(2);
    expect(component.repeaterValues.emit).toHaveBeenCalledWith(
      [
        { keyControl: 'name', operatorControl: 'is not', valueControl: 'testing' },
        { keyControl: 'kind', operatorControl: 'is', valueControl: null },
      ]
    );
    expect(component.formArray.value).toEqual(
      [
        { keyControl: 'name', operatorControl: 'is not', valueControl: 'testing' },
        { keyControl: 'kind', operatorControl: 'is', valueControl: null },
      ]
    );

    // Selecting one of the multipicker options
    const valueKey = fixture.debugElement.queryAll(By.css('.repeater-value'));
    expect(valueKey.length).toBe(2);
    valueKey[1].nativeElement.children[0].children[0].click();
    fixture.detectChanges();
    options = valueKey[1].queryAll(By.css('.ui-multiselect-item'));
    expect(options.length).toBe(3);
    options[0].nativeElement.click();
    fixture.detectChanges();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith(
      [
        { keyControl: 'name', operatorControl: 'is not', valueControl: 'testing' },
        { keyControl: 'kind', operatorControl: 'is', valueControl: ['Node'] },
      ]
    );
    expect(component.formArray.value).toEqual(
      [
        { keyControl: 'name', operatorControl: 'is not', valueControl: 'testing' },
        { keyControl: 'kind', operatorControl: 'is', valueControl: ['Node'] },
      ]
    );
  });

// Same test as above, but data is passed in as form
// groups instead of form controls
it('should load form group data and set correct values-2', () => {
    // Create two repeaters
    component.keyFormName = 'keyControl';
    component.operatorFormName = 'operatorControl';
    component.valueFormName = 'valueControl';
    component.formArray = new FormArray([
      new FormGroup(
        {
          keyControl: new FormControl('name'),
          operatorControl: new FormControl('is not'),
          valueControl: new FormControl('testing')
        }
      ),
      new FormGroup(
        {
          keyControl: new FormControl('severity'),
          operatorControl: new FormControl('is'),
          valueControl: new FormControl('critical')
        }
      )
    ]);
    spyOn(component.repeaterValues, 'emit');
    fixture.detectChanges();

    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith(
      [
        { keyControl: 'name', operatorControl: 'is not', valueControl: 'testing' },
        { keyControl: 'severity', operatorControl: 'is', valueControl: 'critical' },
      ]
    );

    // Changing key
    // Since we have two repeaters, the number of key options should be three since keys are not unique
    const repeaterKey = fixture.debugElement.queryAll(By.css('.repeater-key'));
    expect(repeaterKey.length).toBe(2);
    repeaterKey[1].nativeElement.children[0].children[0].click();
    fixture.detectChanges();
    let options = repeaterKey[1].queryAll(By.css('.ui-dropdown-item'));
    expect(options.length).toBe(3);
    options[1].nativeElement.click(); // selecting second one
    fixture.detectChanges();

    expect(component.repeaterValues.emit).toHaveBeenCalledTimes(2);
    expect(component.repeaterValues.emit).toHaveBeenCalledWith(
      [
        { keyControl: 'name', operatorControl: 'is not', valueControl: 'testing' },
        { keyControl: 'kind', operatorControl: 'is', valueControl: null },
      ]
    );
    expect(component.formArray.value).toEqual(
      [
        { keyControl: 'name', operatorControl: 'is not', valueControl: 'testing' },
        { keyControl: 'kind', operatorControl: 'is', valueControl: null },
      ]
    );

    // Selecting one of the multipicker options
    const valueKey = fixture.debugElement.queryAll(By.css('.repeater-value'));
    expect(valueKey.length).toBe(2);
    valueKey[1].nativeElement.children[0].children[0].click();
    fixture.detectChanges();
    options = valueKey[1].queryAll(By.css('.ui-multiselect-item'));
    expect(options.length).toBe(3);
    options[0].nativeElement.click();
    fixture.detectChanges();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith(
      [
        { keyControl: 'name', operatorControl: 'is not', valueControl: 'testing' },
        { keyControl: 'kind', operatorControl: 'is', valueControl: ['Node'] },
      ]
    );
    expect(component.formArray.value).toEqual(
      [
        { keyControl: 'name', operatorControl: 'is not', valueControl: 'testing' },
        { keyControl: 'kind', operatorControl: 'is', valueControl: ['Node'] },
      ]
    );
  });

  // Tests that data passed in that is invalid
  // throws an error
  it('should throw validation errors', () => {
    component.keyFormName = 'keyControl';
    component.operatorFormName = 'operatorControl';
    component.valueFormName = 'valueControl';
    component.formArray = new FormArray([
      new FormControl({ keyControl: null, operatorControl: 'is not', valueControl: 'testing' }),
    ]);
    expect(() => { component.ngOnInit(); }).toThrowError('key cannot be blank');

    component.formArray = new FormArray([
      new FormControl({ keyControl: 'type', operatorControl: 'is not', valueControl: 'testing' }),
    ]);
    expect(() => { component.ngOnInit(); }).toThrowError('key is not part of the known keys supplied');

    component.formArray = new FormArray([
      new FormControl({ keyControl: 'name', operatorControl: 'equal', valueControl: null }),
    ]);
    expect(() => { component.ngOnInit(); }).toThrowError('operator is not part of the known operators supplied');

    component.formArray = new FormArray([
      new FormControl({ keyControl: 'kind', operatorControl: 'is', valueControl: ['workload'] }),
    ]);
    expect(() => { component.ngOnInit(); }).toThrowError('value is not part of the known values supplied');

    component.formArray = new FormArray([
      new FormControl({ keyControl: 'severity', operatorControl: 'is', valueControl: 'notification' }),
    ]);
    expect(() => { component.ngOnInit(); }).toThrowError('value is not part of the known values supplied');
  });

  it('should have onblur function working', () => {
    component.keyFormName = 'keyControl';
    component.operatorFormName = 'operatorControl';
    component.valueFormName = 'valueControl';
    component.formArray = new FormArray([
      new FormGroup(
        {
          keyControl: new FormControl('name'),
          operatorControl: new FormControl('is not'),
          valueControl: new FormControl('testing')
        }
      ),
      new FormGroup(
        {
          keyControl: new FormControl('severity'),
          operatorControl: new FormControl('is'),
          valueControl: new FormControl('critical')
        }
      )
    ]);
    component.customKeyOnBlur = ($event) => {
      console.log('hello');
    };
    component.customValueOnBlur = ($event) => {
      console.log('world');
    };

    spyOn(component, 'customValueOnBlur');


    fixture.detectChanges();
    const keyInputs = fixture.debugElement.queryAll(By.css('.repeater-input-key'));
    const valueInputs = fixture.debugElement.queryAll(By.css('.repeater-input-value'));
    expect(valueInputs.length).toBe(1);
    expect(keyInputs.length).toBe(0);

    sendClick(valueInputs[0], fixture);
    sendBlur(valueInputs[0], fixture);
    fixture.detectChanges();

    expect(component.customValueOnBlur).toHaveBeenCalled();
  });
});
