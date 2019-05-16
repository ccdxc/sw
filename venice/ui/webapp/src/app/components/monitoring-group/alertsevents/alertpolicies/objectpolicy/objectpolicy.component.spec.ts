import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { ObjectpolicyComponent } from './objectpolicy.component';

describe('ObjectpolicyComponent', () => {
  let component: ObjectpolicyComponent;
  let fixture: ComponentFixture<ObjectpolicyComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [ObjectpolicyComponent]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(ObjectpolicyComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
