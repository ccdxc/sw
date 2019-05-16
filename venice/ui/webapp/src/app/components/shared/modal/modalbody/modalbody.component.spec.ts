import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { ModalbodyComponent } from './modalbody.component';

describe('ModalbodyComponent', () => {
  let component: ModalbodyComponent;
  let fixture: ComponentFixture<ModalbodyComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [ModalbodyComponent],
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(ModalbodyComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
