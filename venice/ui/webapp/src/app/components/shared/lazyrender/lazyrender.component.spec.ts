import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { LazyrenderComponent } from './lazyrender.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

describe('LazyrenderComponent', () => {
  let component: LazyrenderComponent;
  let fixture: ComponentFixture<LazyrenderComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [LazyrenderComponent],
      imports: [
        MaterialdesignModule,
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(LazyrenderComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
