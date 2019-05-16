import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { ModalwidgetComponent } from './modalwidget.component';

describe('ModalwidgetComponent', () => {
  let component: ModalwidgetComponent;
  let fixture: ComponentFixture<ModalwidgetComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [ModalwidgetComponent],
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(ModalwidgetComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
