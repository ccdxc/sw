import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ReactiveFormsModule } from '@angular/forms';
import { RepeaterComponent } from '@modules/widgets/repeater/repeater.component';
import { PrimengModule } from '../../primeng.module';
import { MaterialModule } from '../../material.module';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { TestRepeaterComponent } from '@testapp/test-repeater/test-repeater.component';

describe('TestRepeaterComponent', () => {
  let component: TestRepeaterComponent;
  let fixture: ComponentFixture<TestRepeaterComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [TestRepeaterComponent, RepeaterComponent],
      imports: [ReactiveFormsModule, PrimengModule, MaterialModule, NoopAnimationsModule]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TestRepeaterComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
