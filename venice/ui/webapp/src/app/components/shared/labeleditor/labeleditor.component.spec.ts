import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { LabeleditorComponent } from './labeleditor.component';
import {RouterTestingModule} from '@angular/router/testing';
import {HttpClientTestingModule} from '@angular/common/http/testing';
import {NoopAnimationsModule} from '@angular/platform-browser/animations';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {PrimengModule} from '@lib/primeng.module';
import {FormsModule} from '@angular/forms';
import {ClusterDistributedServiceCard} from '@sdk/v1/models/generated/cluster';
import { configureTestSuite } from 'ng-bullet';
import {By} from '@angular/platform-browser';
import {TestingUtility} from '@common/TestingUtility';

describe('LabeleditorComponent', () => {
  let component: LabeleditorComponent;
  let fixture: ComponentFixture<LabeleditorComponent>;
  let tu: TestingUtility;
  const naples1 = {
    'meta': {
      'name': 'naples1',
      'labels': {
        'Location': 'us-west-A'
      },
      'mod-time': '2018-08-23T17:35:08.534909931Z',
      'creation-time': '2018-08-23T17:30:08.534909931Z'
    },
    'spec': {
      'id': 'naples1-host'
    },
    'status': {
      'ip-config': {
        'ip-address': '0.0.0.0/0'
      },
      'primary-mac': '00ae.cd00.1142',
      'admission-phase': 'PENDING',
      'DSCVersion': '1.0E',
    }
  };

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [LabeleditorComponent],
      imports: [
        RouterTestingModule,
        HttpClientTestingModule,
        NoopAnimationsModule,
        MaterialdesignModule,
        PrimengModule,
        FormsModule,
      ],
      providers: []
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(LabeleditorComponent);
    component = fixture.componentInstance;
    component.objects = [];
    component.objects.push(new ClusterDistributedServiceCard(naples1));
    component.inLabelEditMode = true;
    tu = new TestingUtility(fixture);
    fixture.detectChanges();
  });

  it('should load labels', () => {
    component.ngOnChanges(null);
    expect(component.models[0].key).toBe('Location');
    expect(component.models[0].value).toBe('us-west-A');
  });

  it('should have add and delete button working properly', () => {
    component.ngOnChanges(null);
    fixture.detectChanges();
    let deleteBtns = fixture.debugElement.queryAll(By.css('.label-editor-delete'));
    let addBtns = fixture.debugElement.queryAll(By.css('.label-editor-add'));

    expect(deleteBtns).toBeTruthy();
    expect(addBtns).toBeTruthy();

    // click the first add btn
    tu.sendClick(addBtns[0]);
    fixture.detectChanges();

    const keyInputs = fixture.debugElement.queryAll(By.css('.label-editor-input-key'));
    const valueInputs = fixture.debugElement.queryAll(By.css('.label-editor-input-value'));

    expect(keyInputs.length).toBe(2);
    expect(valueInputs.length).toBe(2);

    // set hello world to the second row of key and value
    tu.setText(keyInputs[1], 'hello');
    tu.setText(valueInputs[1], 'world');

    expect(component.models.length).toBe(2);
    expect(component.models[1].key).toBe('hello');
    expect(component.models[1].value).toBe('world');

    // click the first delete btn to delete the first row
    deleteBtns = fixture.debugElement.queryAll(By.css('.label-editor-delete'));
    addBtns = fixture.debugElement.queryAll(By.css('.label-editor-add'));

    tu.sendClick(deleteBtns[0]);
    fixture.detectChanges();
    // after we delete the original row, the only thing left is the hello world we added
    expect(component.models.length).toBe(1);
    expect(component.models[0].key).toBe('hello');
    expect(component.models[0].value).toBe('world');
  });

  it('should reset the field after cancel', () => {
    component.ngOnChanges(null);
    fixture.detectChanges();
    const deleteBtns = fixture.debugElement.queryAll(By.css('.label-editor-delete'));
    const addBtns = fixture.debugElement.queryAll(By.css('.label-editor-add'));

    expect(deleteBtns).toBeTruthy();
    expect(addBtns).toBeTruthy();

    // click the first add btn
    tu.sendClick(addBtns[0]);
    fixture.detectChanges();

    const keyInputs = fixture.debugElement.queryAll(By.css('.label-editor-input-key'));
    const valueInputs = fixture.debugElement.queryAll(By.css('.label-editor-input-value'));

    // set hello world to the second row of key and value
    tu.setText(keyInputs[1], 'hello');
    tu.setText(valueInputs[1], 'world');

    const cancelBtn = fixture.debugElement.query(By.css('.label-editor-cancel'));
    expect(cancelBtn).toBeTruthy();

    // after we cancel, the data should be restored
    tu.sendClick(cancelBtn);
    expect(component.models[0].key).toBe('Location');
    expect(component.models[0].value).toBe('us-west-A');
  });
});
