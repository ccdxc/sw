import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { MatchruleComponent } from './matchrule.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

describe('MatchruleComponent', () => {
  let component: MatchruleComponent;
  let fixture: ComponentFixture<MatchruleComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [MatchruleComponent],
      imports: [
        MaterialdesignModule,
        PrimengModule,
        ReactiveFormsModule,
        FormsModule,
        NoopAnimationsModule,
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(MatchruleComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
