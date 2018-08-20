import { ComponentFixture, TestBed, async } from '@angular/core/testing';
import { FormArray, FormControl, FormGroup, FormsModule, ReactiveFormsModule } from '@angular/forms';
import { By } from '@angular/platform-browser';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
// import { MultiselectComponent } from '@modules/widgets/multiselect/multiselect.component';
import { MaterialModule } from '../../../../material.module';
import { PrimengModule } from '../../../../primeng.module';
import { ValueType } from '../index';
import { UniquerepeaterComponent } from './uniquerepeater.component';


describe('UniquerepeaterComponent', () => {
  let component: UniquerepeaterComponent;
  let fixture: ComponentFixture<UniquerepeaterComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [UniquerepeaterComponent],
      imports: [FormsModule, ReactiveFormsModule, PrimengModule, MaterialModule, NoopAnimationsModule]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(UniquerepeaterComponent);
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
    ]
  });

  afterEach(() => {
    TestBed.resetTestingModule();
  });

  it('should add and delete unique repeaters', () => {
    // should emit a default value
    const spy = spyOn(component.repeaterValues, 'emit');
    fixture.detectChanges();
    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith([{ keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' }]);
    expect(component.repeaterList.length).toBe(1);

    // trash icon should not be visible
    let trashElem = fixture.debugElement.queryAll(By.css('.uniquerepeater-trash-icon'));
    expect(trashElem.length).toBe(0);
    // Add icon should be visible
    let addElem = fixture.debugElement.queryAll(By.css('.uniquerepeater-and'));
    expect(addElem.length).toBe(1);
    spy.calls.reset();

    // adding another repeater value
    addElem[0].nativeElement.querySelector('span').click();
    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith([{ keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' }, { keyFormControl: 'kind', operatorFormControl: 'is', valueFormControl: null }]);
    expect(component.repeaterList.length).toBe(2);
    spy.calls.reset();
    fixture.detectChanges();

    // one And, two trash cans
    addElem = fixture.debugElement.queryAll(By.css('.uniquerepeater-and'));
    trashElem = fixture.debugElement.queryAll(By.css('.uniquerepeater-trash-icon'));
    expect(addElem.length).toBe(1);
    expect(trashElem.length).toBe(2);

    // last add
    addElem[0].nativeElement.querySelector('span').click();
    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith([
      { keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' },
      { keyFormControl: 'kind', operatorFormControl: 'is', valueFormControl: null },
      { keyFormControl: 'name', operatorFormControl: 'is', valueFormControl: null },
    ]);
    expect(component.repeaterList.length).toBe(3);
    spy.calls.reset();
    fixture.detectChanges();

    // zero Ands, three trash cans
    addElem = fixture.debugElement.queryAll(By.css('.uniquerepeater-and'));
    trashElem = fixture.debugElement.queryAll(By.css('.uniquerepeater-trash-icon'));
    expect(addElem.length).toBe(0);
    expect(trashElem.length).toBe(3);

    // delete
    trashElem[0].nativeElement.children[0].click();
    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith([
      { keyFormControl: 'kind', operatorFormControl: 'is', valueFormControl: null },
      { keyFormControl: 'name', operatorFormControl: 'is', valueFormControl: null },
    ]);
    expect(component.repeaterList.length).toBe(2);
    spy.calls.reset();
    fixture.detectChanges();

    // one And, two trash cans
    addElem = fixture.debugElement.queryAll(By.css('.uniquerepeater-and'));
    trashElem = fixture.debugElement.queryAll(By.css('.uniquerepeater-trash-icon'));
    expect(addElem.length).toBe(1);
    expect(trashElem.length).toBe(2);


    // delete
    trashElem[0].nativeElement.children[0].click();
    expect(component.repeaterValues.emit).toHaveBeenCalled();
    expect(component.repeaterValues.emit).toHaveBeenCalledWith([
      { keyFormControl: 'name', operatorFormControl: 'is', valueFormControl: null },
    ]);
    expect(component.repeaterList.length).toBe(1);
    spy.calls.reset();
    fixture.detectChanges();

    // one And, two trash cans
    addElem = fixture.debugElement.queryAll(By.css('.uniquerepeater-and'));
    trashElem = fixture.debugElement.queryAll(By.css('.uniquerepeater-trash-icon'));
    expect(component.repeaterList.length).toBe(1);
    expect(addElem.length).toBe(1);
    expect(trashElem.length).toBe(0);
  });

  it('should load form control data and set correct values', () => {
    // Create two repeaters
    component.keyFormName = 'keyControl';
    component.operatorFormName = 'operatorControl';
    component.valueFormName = 'valueControl'
    component.formArray = new FormArray([
      new FormControl({ keyControl: 'name', operatorControl: 'is not', valueControl: 'testing' }),
      new FormControl({ keyControl: 'severity', operatorControl: 'is', valueControl: 'critical' })
    ])
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
    // Since we have two repeaters, the number of key options should be two
    let repeaterKey = fixture.debugElement.queryAll(By.css('.uniquerepeater-key'));
    expect(repeaterKey.length).toBe(2);
    repeaterKey[1].nativeElement.children[0].children[0].click();
    fixture.detectChanges();
    let options = repeaterKey[1].queryAll(By.css('.ui-dropdown-item'));
    expect(options.length).toBe(2);
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
    let valueKey = fixture.debugElement.queryAll(By.css('.uniquerepeater-value'));
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
  it('should load form group data and set correct values', () => {
    // Create two repeaters
    component.keyFormName = 'keyControl';
    component.operatorFormName = 'operatorControl';
    component.valueFormName = 'valueControl'
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
    // Since we have two repeaters, the number of key options should be two
    let repeaterKey = fixture.debugElement.queryAll(By.css('.uniquerepeater-key'));
    expect(repeaterKey.length).toBe(2);
    repeaterKey[1].nativeElement.children[0].children[0].click();
    fixture.detectChanges();
    let options = repeaterKey[1].queryAll(By.css('.ui-dropdown-item'));
    expect(options.length).toBe(2);
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
    let valueKey = fixture.debugElement.queryAll(By.css('.uniquerepeater-value'));
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
    component.valueFormName = 'valueControl'
    component.formArray = new FormArray([
      new FormControl({ keyControl: null, operatorControl: 'is not', valueControl: 'testing' }),
    ])
    expect(() => { component.ngOnInit() }).toThrowError('key cannot be blank');

    component.formArray = new FormArray([
      new FormControl({ keyControl: 'type', operatorControl: 'is not', valueControl: 'testing' }),
    ])
    expect(() => { component.ngOnInit() }).toThrowError('key is not part of the known keys supplied');

    component.formArray = new FormArray([
      new FormControl({ keyControl: 'name', operatorControl: 'equal', valueControl: null }),
    ])
    expect(() => { component.ngOnInit() }).toThrowError('operator is not part of the known operators supplied');

    component.formArray = new FormArray([
      new FormControl({ keyControl: 'kind', operatorControl: 'is', valueControl: ['workload'] }),
    ])
    expect(() => { component.ngOnInit() }).toThrowError('value is not part of the known values supplied');

    component.formArray = new FormArray([
      new FormControl({ keyControl: 'severity', operatorControl: 'is', valueControl: 'notification' }),
    ])
    expect(() => { component.ngOnInit() }).toThrowError('value is not part of the known values supplied');
  })

});
