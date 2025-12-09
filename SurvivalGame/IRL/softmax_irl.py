# softmax_irl.py
import json
import torch
from torch import nn
from torch.optim import Adam

class SoftmaxIRL:
    def __init__(self, K, lr=0.05, device="cpu"):
        """
        K: 목적 함수 개수
        lr: learning rate
        """
        self.K = K
        self.device = device

        # w는 K차원 파라미터 (초기값: uniform)
        init_w = torch.ones(K, dtype=torch.float32) / K
        # simplex 제약 (w >= 0, sum w = 1)을 나중에 projection으로 처리할거라
        # 여기선 그냥 unconstrained param을 둔다.
        self.w_raw = nn.Parameter(init_w.clone())

        self.optimizer = Adam([self.w_raw], lr=lr)

    def project_simplex(self, v):
        """
        v를 확률 simplex (v_i >= 0, sum v_i = 1)로 projection
        (standard algorithm)
        """
        # v: (K,)
        u, _ = torch.sort(v, descending=True)
        cssv = torch.cumsum(u, dim=0)
        rho = torch.nonzero(u * torch.arange(1, len(u)+1, device=v.device) > (cssv - 1), as_tuple=False)
        rho = rho[-1, 0].item()
        theta = (cssv[rho] - 1) / float(rho + 1)
        w = torch.clamp(v - theta, min=0.0)
        return w

    def load_demo(self, json_path):
        with open(json_path, "r") as f:
            data = json.load(f)

        self.objective_ids = data["objective_ids"]
        steps = data["steps"]

        # sub_rewards_list: list of (num_actions, K)
        # chosen_indices: list of chosen_action_index
        sub_rewards_list = []
        chosen_indices = []

        for step in steps:
            sub_rewards = torch.tensor(step["sub_rewards"], dtype=torch.float32)  # (A, K)
            chosen = step["chosen_action_index"]

            sub_rewards_list.append(sub_rewards)
            chosen_indices.append(chosen)

        return sub_rewards_list, chosen_indices

    def compute_loss(self, sub_rewards_list, chosen_indices):
        """
        L_demo(w; tau) = - sum_t log pi(a_t | s_t; w)
        """
        w = self.project_simplex(self.w_raw)  # (K,)
        total_log_prob = 0.0
        total_steps = 0

        for sub_rewards, chosen in zip(sub_rewards_list, chosen_indices):
            # sub_rewards: (A, K)
            # 행동 값: q_a = w^T r(s,a)
            # (A,) = (A,K) @ (K,)
            q = sub_rewards @ w  # (A,)

            # softmax로 정책
            log_probs = torch.log_softmax(q, dim=0)  # (A,)

            total_log_prob += log_probs[chosen]
            total_steps += 1

        # 평균 negative log-likelihood
        loss = - total_log_prob / total_steps
        return loss

    def fit(self, sub_rewards_list, chosen_indices, num_steps=500, print_every=50):
        for step in range(1, num_steps+1):
            self.optimizer.zero_grad()
            loss = self.compute_loss(sub_rewards_list, chosen_indices)
            loss.backward()
            self.optimizer.step()

            if step % print_every == 0:
                with torch.no_grad():
                    w_proj = self.project_simplex(self.w_raw)
                print(f"[SoftmaxIRL] step={step}, loss={loss.item():.4f}, w={w_proj.tolist()}")

        with torch.no_grad():
            w_final = self.project_simplex(self.w_raw)
        return w_final

def main():
    import argparse

    # parser = argparse.ArgumentParser()
    # parser.add_argument("--demo_json", type=str, required=True)
    # parser.add_argument("--lr", type=float, default=0.05)
    # parser.add_argument("--steps", type=int, default=500)
    # parser.add_argument("--out_json", type=str, default="estimated_weights_softmax.json")
    # args = parser.parse_args()

    # 데모 로드
    # tmp = json.load(open(args.trajectory, "r"))
    demo_json = "IRLRecord_20251210_002257.json"

    lr = 0.05
    steps=10000
    out_json = "estimated_weights_softmax.json"

    tmp = json.load(open(demo_json, "r"))

    K = tmp["K"]

    irl = SoftmaxIRL(K=K, lr=lr)
    sub_rewards_list, chosen_indices = irl.load_demo(demo_json)

    w = irl.fit(sub_rewards_list, chosen_indices, num_steps=steps)

    # UE에서 쓸 수 있도록 objective_id → weight 매핑 JSON으로 내보내기
    result = {
        "objective_ids": tmp["objective_ids"],
        "weights": w.tolist()  # 순서대로 매핑
    }
    with open(out_json, "w") as f:
        json.dump(result, f, indent=2)

    print("[SoftmaxIRL] Final weights saved to", out_json)


