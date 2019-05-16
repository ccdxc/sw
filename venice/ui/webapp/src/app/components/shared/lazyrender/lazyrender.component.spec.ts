import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { LazyrenderComponent } from './lazyrender.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

describe('LazyrenderComponent', () => {
  let component: LazyrenderComponent;
  let fixture: ComponentFixture<LazyrenderComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [LazyrenderComponent],
      imports: [
        MaterialdesignModule,
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(LazyrenderComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
