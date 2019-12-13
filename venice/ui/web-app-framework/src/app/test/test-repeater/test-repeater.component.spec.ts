import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { TestRepeaterComponent } from './test-repeater.component';
import { ReactiveFormsModule } from '@angular/forms';
import { RepeaterComponent } from '@modules/widgets/repeater/repeater.component';
import { PrimengModule } from '../../primeng.module';
import { MaterialModule } from '../../material.module';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { LabelDirective } from '@modules/widgets/directives/label.directive';

describe('TestRepeaterComponent', () => {
  let component: TestRepeaterComponent;
  let fixture: ComponentFixture<TestRepeaterComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [TestRepeaterComponent, RepeaterComponent, LabelDirective],
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
