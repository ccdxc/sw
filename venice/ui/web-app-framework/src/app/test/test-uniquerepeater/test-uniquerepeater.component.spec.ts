import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { TestUniqueRepeaterComponent } from './test-uniquerepeater.component';
import { ReactiveFormsModule } from '@angular/forms';
import { UniquerepeaterComponent } from '@modules/widgets/repeater/uniquerepeater/uniquerepeater.component';
import { PrimengModule } from '../../primeng.module';
import { MaterialModule } from '../../material.module';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

describe('TestUniqueRepeaterComponent', () => {
  let component: TestUniqueRepeaterComponent;
  let fixture: ComponentFixture<TestUniqueRepeaterComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [TestUniqueRepeaterComponent, UniquerepeaterComponent],
      imports: [ReactiveFormsModule, PrimengModule, MaterialModule, NoopAnimationsModule]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TestUniqueRepeaterComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
