import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { FieldselectorComponent } from './fieldselector.component';
import { WidgetsModule } from 'web-app-framework';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

describe('FieldselectorComponent', () => {
  let component: FieldselectorComponent;
  let fixture: ComponentFixture<FieldselectorComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [FieldselectorComponent],
      imports: [
        WidgetsModule,
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(FieldselectorComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
