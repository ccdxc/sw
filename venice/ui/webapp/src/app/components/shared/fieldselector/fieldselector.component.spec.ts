import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { FieldselectorComponent } from './fieldselector.component';
import { WidgetsModule } from 'web-app-framework';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

describe('FieldselectorComponent', () => {
  let component: FieldselectorComponent;
  let fixture: ComponentFixture<FieldselectorComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [FieldselectorComponent],
      imports: [
        WidgetsModule,
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FieldselectorComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
